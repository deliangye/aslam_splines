/*
 * DiffManifoldBSpline.hpp
 *
 *  Created on: Apr 23, 2012
 *      Author: Hannes Sommer
 */

#ifndef RIEMANNIANBSPLINE_HPP_
#define RIEMANNIANBSPLINE_HPP_

#include "ExternalIncludes.hpp"
#include "DynamicOrTemplateInt.hpp"
#include "SimpleTypeTimePolicy.hpp"
#include "manifolds/DiffManifold.hpp"
#include "KnotArithmetics.hpp"

#include "gtest/gtest_prod.h"

namespace bsplines {
	typedef SimpleTypeTimePolicy<double> DefaultTimePolicy;

	template <typename TDiffManifoldBSplineConfiguration, typename TConfigurationDerived = TDiffManifoldBSplineConfiguration>
	class DiffManifoldBSpline{
	};

	template <typename TDiffManifoldConfiguration, int ISplineOrder = Eigen::Dynamic, typename TTimePolicy = DefaultTimePolicy>
	struct DiffManifoldBSplineConfiguration : public TDiffManifoldConfiguration {
	public:
		typedef DiffManifoldBSplineConfiguration<TDiffManifoldConfiguration, ISplineOrder, TTimePolicy> Conf;

		typedef eigenTools::DynamicOrTemplateInt<ISplineOrder> SplineOrder;

		typedef DiffManifoldBSpline<Conf> BSpline;
		typedef TDiffManifoldConfiguration ManifoldConf;
		typedef TTimePolicy TimePolicy;

		DiffManifoldBSplineConfiguration(TDiffManifoldConfiguration manifoldConfiguration, int splineOrder = ISplineOrder) : TDiffManifoldConfiguration(manifoldConfiguration), _splineOrder(splineOrder) {}

		const SplineOrder getSplineOrder() const { return _splineOrder; }

	private:
		const SplineOrder _splineOrder;
	};


	namespace internal {
		template <typename TDiffManifoldBSplineConfiguration>
		struct DiffManifoldBSplineTraits {
			enum { NeedsCumulativeBasisMatrices = false };
		};

		template <typename TDiffManifoldBSplineConfiguration>
		struct SegmentData {
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
			typedef typename TDiffManifoldBSplineConfiguration::Manifold Manifold;
			typedef typename Manifold::point_t point_t;
			typedef typename TDiffManifoldBSplineConfiguration::TimePolicy::time_t time_t;
			typedef Eigen::Matrix<typename Manifold::scalar_t, TDiffManifoldBSplineConfiguration::SplineOrder::VALUE, TDiffManifoldBSplineConfiguration::SplineOrder::VALUE> basis_matrix_t;

		protected:
			point_t _point;
			basis_matrix_t _basisMatrix;
		public:
			inline point_t & getControlVertex() { return _point; }
			inline basis_matrix_t & getBasisMatrix() { return _basisMatrix; }

			inline void invalidateCache();
			inline void setControlVertex(const point_t & point) { _point = point; invalidateCache(); }

			inline const point_t & getControlVertex() const { return _point; }
			inline const basis_matrix_t & getBasisMatrix() const { return _basisMatrix; }

			inline SegmentData(const TDiffManifoldBSplineConfiguration & conf, const time_t & t, const point_t & point) : _point(point), _basisMatrix((int)conf.getSplineOrder(), (int)conf.getSplineOrder()) {}
		};

		template <typename TDiffManifoldBSplineConfiguration>
		struct SegmentMap : public std::map<typename TDiffManifoldBSplineConfiguration::TimePolicy::time_t, SegmentData<TDiffManifoldBSplineConfiguration> > {
			typedef std::map<typename TDiffManifoldBSplineConfiguration::TimePolicy::time_t, SegmentData<TDiffManifoldBSplineConfiguration> > parent_t;
			typedef SegmentData<TDiffManifoldBSplineConfiguration> segment_data_t;
			typedef typename segment_data_t::point_t point_t;
			typedef typename segment_data_t::basis_matrix_t basis_matrix_t;
			typedef typename TDiffManifoldBSplineConfiguration::TimePolicy::time_t time_t;
			typedef typename std::map<time_t, SegmentData<TDiffManifoldBSplineConfiguration> > segment_map_t;
			typedef typename segment_map_t::iterator SegmentMapIterator;
			typedef typename segment_map_t::const_iterator SegmentMapConstIterator;

			template<typename SEGMENTDATA, typename BASE>
			struct SegmentIteratorT : public BASE {
				SegmentIteratorT(){}
				SegmentIteratorT(const BASE & it) : BASE(it) {}
				inline const time_t & getKnot() const { return (* static_cast<const BASE *>(this))->first; }
				inline const time_t & getTime() const { return getKnot(); }
				inline SEGMENTDATA & operator *() const { return (* static_cast<const BASE *>(this))->second; }
				inline SEGMENTDATA * operator ->() const { return &(* static_cast<const BASE *>(this))->second; }
			};

			struct SegmentConstIterator : public SegmentIteratorT<const segment_data_t, SegmentMapConstIterator> {
				SegmentConstIterator(){}
				SegmentConstIterator(const SegmentMapIterator & it) : SegmentIteratorT<const segment_data_t, SegmentMapConstIterator>(it) {}
				SegmentConstIterator(const SegmentMapConstIterator & it) : SegmentIteratorT<const segment_data_t, SegmentMapConstIterator>(it) {}
			};

			struct SegmentIterator : public SegmentIteratorT<segment_data_t, SegmentMapIterator> {
				SegmentIterator(){}
				SegmentIterator(const SegmentMapIterator & it) : SegmentIteratorT<segment_data_t, SegmentMapIterator>(it) {}
			};
		};
	}

	template <typename TSpline> class BSplineFitter;

	template <typename TDiffManifoldConfiguration, int ISplineOrder, typename TTimePolicy, typename TConfigurationDerived>
	class DiffManifoldBSpline<DiffManifoldBSplineConfiguration<TDiffManifoldConfiguration, ISplineOrder, TTimePolicy>, TConfigurationDerived> {
	public:
		SM_DEFINE_EXCEPTION(Exception, std::runtime_error);

		typedef TConfigurationDerived configuration_t;
		typedef typename configuration_t::BSpline spline_t;
		typedef typename configuration_t::Manifold manifold_t;
		typedef typename manifold_t::scalar_t scalar_t;
		typedef typename manifold_t::point_t point_t;
		typedef typename manifold_t::tangent_vector_t tangent_vector_t;
		typedef typename manifold_t::dmatrix_t dmatrix_t;

		typedef typename configuration_t::TimePolicy TimePolicy;
		typedef typename TimePolicy::time_t time_t;
		typedef typename TimePolicy::duration_t duration_t;

		enum {
			SplineOrder = configuration_t::SplineOrder::VALUE,
			Dimension = manifold_t::Dimension,
			PointSize = manifold_t::PointSize,
			NeedsCumulativeBasisMatrices = internal::DiffManifoldBSplineTraits<TConfigurationDerived>::NeedsCumulativeBasisMatrices
		};

		typedef Eigen::Matrix<scalar_t, manifold_t::PointSize, multiplyEigenSize(Dimension, SplineOrder) > full_jacobian_t;
		typedef Eigen::Matrix<scalar_t, SplineOrder, 1> SplineOrderVector;
		typedef Eigen::Matrix<scalar_t, SplineOrder, SplineOrder> SplineOrderSquareMatrix;

	public:
		typedef internal::SegmentMap<configuration_t> segment_map_t;
		typedef typename segment_map_t::segment_data_t segment_data_t;

	protected:
		typedef typename segment_map_t::SegmentMapIterator SegmentMapIterator;
		typedef typename segment_map_t::SegmentMapConstIterator SegmentMapConstIterator;

	public:
		typedef typename segment_map_t::SegmentIterator SegmentIterator;
		typedef typename segment_map_t::SegmentConstIterator SegmentConstIterator;


		DiffManifoldBSpline(const configuration_t & configuration) : _configuration(configuration), _manifold(configuration), _segments(new segment_map_t()){}

		inline typename configuration_t::SplineOrder getSplineOrder() const { return _configuration.getSplineOrder(); }
		inline typename configuration_t::Dimension getDimension() const { return _configuration.getDimension(); }
		inline typename configuration_t::PointSize getPointSize() const { return _configuration.getPointSize(); }

		inline const manifold_t & getManifold() const;

		//TODO optimize : enable returning reference
		inline const SegmentConstIterator getAbsoluteBegin() const;

		inline const SegmentIterator getAbsoluteBegin();

		inline const SegmentConstIterator getAbsoluteEnd() const;

		inline const SegmentIterator getAbsoluteEnd();

		int minimumKnotsRequired() const;

		inline size_t getAbsoluteNumberOfSegments() const;

		inline void addKnot(const time_t & time);

		void addControlVertex(const time_t & time, const point_t & point);

		void init();

		void initConstantUniformSpline(const time_t & t_min, const time_t & t_max, int numSegments, const point_t & constant);

		inline int getNumValidTimeSegments() const;

		inline int getNumControlVertices() const;

		/**
		 * get the number of knots in this slice
		 * @return
		 */
		inline size_t getNumKnots() const;

		inline time_t getMinTime() const;
		inline time_t getMaxTime() const;

		inline std::pair<time_t, time_t >getTimeInterval() const { return std::pair<time_t, time_t> (getMinTime(), getMaxTime()); };

		inline SegmentIterator firstRelevantSegment();
		inline SegmentConstIterator firstRelevantSegment() const;
		inline SegmentIterator begin();
		inline SegmentConstIterator begin() const;
		inline SegmentIterator end();
		inline SegmentConstIterator end() const;

		void computeDiiInto(const SegmentConstIterator & segmentIt, SplineOrderSquareMatrix & D) const;

		void computeViInto(SegmentConstIterator segmentIt, SplineOrderSquareMatrix & V) const;

		/**
		 * find the greatest knot less or equal to t
		 * @param t time
		 * @return knot iterator
		 */
		inline SegmentIterator getSegmentIterator(const time_t & t);
		inline SegmentConstIterator getSegmentIterator(const time_t & t) const;

		inline SegmentConstIterator getFirstRelevantSegmentByLast(const SegmentConstIterator & first) const;
		inline SegmentIterator getFirstRelevantSegmentByLast(const SegmentIterator & first);

		inline void setLocalCoefficientVector(const time_t & t, const Eigen::VectorXd & coefficients, int pointSize);

		inline void getLocalCoefficientVector(const time_t & t, Eigen::VectorXd & coefficients, int pointSize);


		template <typename TValue, typename TFunctor>
		TValue evalFunctorIntegralNumerically(const time_t & t1, const time_t & t2, const TFunctor & f, int numberOfPoints = 100) const;

		template <typename TValue, typename TFunctor>
		inline TValue evalFunctorIntegral(const time_t & t1, const time_t & t2, const TFunctor & f) const { return evalFunctorIntegralNumerically<TValue, TFunctor>(t1, t2, f); }

		point_t evalIntegralNumerically(const time_t & t1, const time_t & t2, int numberOfPoints = 100) const;
		inline point_t evalIntegral(const time_t & t1, const time_t & t2) const { return evalIntegralNumerically(t1, t2); }
		inline point_t evalI(const time_t & t1, const time_t & t2) const { return evalIntegral(t1, t2); }

		template<int IMaximalDerivativeOrder>
		class Evaluator {
		public:
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW

			Evaluator(const spline_t & spline, const time_t & t);
			SegmentConstIterator getFirstRelevantSegmentIterator() const;
			inline SegmentConstIterator begin() const { return getFirstRelevantSegmentIterator(); }

			SegmentConstIterator getLastRelevantSegmentIterator() const;

			SegmentConstIterator end() const;

			inline time_t getKnot() const;

			inline point_t eval() const;

			inline point_t evalD(int derivativeOrder) const;

			inline point_t evalGeneric() const;

			inline point_t evalDGeneric(int derivativeOrder) const;

			const duration_t getSegmentLength() const;

			const duration_t getPositionInSegment() const;

			double getRelativePositionInSegment() const;

			const spline_t& getSpline() const;

		protected:
			enum { NumberOfPreparedDerivatives = (IMaximalDerivativeOrder <= SplineOrder ? IMaximalDerivativeOrder : SplineOrder) + 1};
			SplineOrderVector _localBi[NumberOfPreparedDerivatives];
			//TODO cache results
			mutable SplineOrderVector _tmp;

			const spline_t & _spline;
			const time_t _t;
			const SegmentConstIterator _ti, _firstRelevantControlVertexIt, _end;
			const duration_t _segmentLength;
			const duration_t _positionInSegment;
			const double _relativePositionInSegment;

			inline void computeLocalViInto(SplineOrderVector & localVi) const;
			inline const SplineOrderVector & getLocalBi(int derivativeOrder = 0) const;
			inline const SplineOrderVector & getLocalCumulativeBi(int derivativeOrder = 0) const;
			inline void computeLocalBiInto(SplineOrderVector & ret, int derivativeOrder = 0) const;
			inline void computeLocalCumulativeBiInto(SplineOrderVector & ret, int derivativeOrder = 0) const;
			inline void computeLocalCumulativeBiInto(const SplineOrderVector & localBi, SplineOrderVector & ret, int derivativeOrder = 0, bool argumentsAreTheSame = false) const;
			inline int dmul(int i, int derivativeOrder) const;
			inline void computeUInto(int derivativeOrder, SplineOrderVector & u) const;
			inline void computeVInto(SplineOrderVector & v) const;

			template<bool BCumulative> inline void computeLocalBiIntoT(SplineOrderVector& ret, int derivativeOrder) const;
			template<bool BCumulative> inline const SplineOrderVector& getLocalBiT(int derivativeOrder = 0) const;

			//friends:
			template<typename TSpline> friend class BSplineFitter;
			FRIEND_TEST(DiffManifoldBSplineTestSuite, testGetBi);
			FRIEND_TEST(DiffManifoldBSplineTestSuite, testInitialization);
			FRIEND_TEST(UnitQuaternionBSplineTestSuite, evalAngularVelocityAndAcceleration);
		};

		template<int IMaximalDerivativeOrder>
		inline Evaluator<IMaximalDerivativeOrder> getEvaluatorAt(const time_t & t) const;

	protected:
		typedef typename KnotArithmetics::UniformTimeCalculator<TimePolicy> UniformTimeCalculator;

		const TConfigurationDerived _configuration;
		const manifold_t _manifold;
		const boost::shared_ptr<segment_map_t> _segments;
		SegmentIterator _begin, _end, _firstRelevantSegment;

		inline const spline_t & getDerived() const { return *static_cast<const spline_t *>(this); }
		inline spline_t & getDerived() { return *static_cast<spline_t *>(this); }

		inline segment_map_t & getSegmentMap();
		inline const segment_map_t & getSegmentMap() const;

		/**
		 * compute duration
		 * @param from
		 * @param till
		 * @return till - from
		 */
		inline static duration_t computeDuration(time_t from, time_t till);

		/**
		 * divide durations
		 * @param a
		 * @param b
		 * @return a/b
		 */
		inline static double divideDurations(duration_t a, duration_t b);

		inline static double getDurationAsDouble(duration_t d);

		void initializeBasisMatrices();

		segment_data_t createSegmentData(const time_t & time, const point_t & point) const;

		Eigen::MatrixXd M(int k, const std::deque<time_t> & knots);

		static double d_0(int k, int i, int j, const std::deque<time_t> & knots);
		static double d_1(int k, int i, int j, const std::deque<time_t> & knots);

		inline duration_t computeSegmentLength(SegmentMapConstIterator segmentIt) const;
		inline static void computeLocalViInto(const SplineOrderVector & v, SplineOrderVector& localVi, const SegmentMapConstIterator & it);

		template<int IMaximalDerivativeOrder> friend class Evaluator;
	};
}

#include "implementation/DiffManifoldBSplineImpl.hpp"

#endif /* RIEMANNIANBSPLINE_HPP_ */