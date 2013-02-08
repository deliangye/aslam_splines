/*
 * EuclideanSpace.hpp
 *
 *  Created on: Jul 26, 2012
 *      Author: hannes
 */

#ifndef FLATRIEMANNIANGEOMETRY_HPP_
#define FLATRIEMANNIANGEOMETRY_HPP_

#include "LieGroup.hpp"

namespace manifolds {

	template <int IDimension = Eigen::Dynamic, typename TScalar = double>
	struct EuclideanSpaceConf : public LieGroupConf<IDimension, IDimension, TScalar> {
		typedef LieGroupConf<IDimension, IDimension, TScalar> ParentConf;
		typedef DiffManifold<ParentConf> ParentManifold;
		typedef EuclideanSpaceConf Conf;
		typedef DiffManifold<Conf> Manifold;

		typename ParentConf::Dimension _dimension;
		EuclideanSpaceConf(int dimension = IDimension) : _dimension(dimension) {}
		typename ParentConf::Dimension getDimension()const { return _dimension; }
		typename ParentConf::Dimension getPointSize()const { return getDimension(); }
	};

	template <int IDimension, typename TScalar, typename TConfigurationDerived>
	class DiffManifold< EuclideanSpaceConf<IDimension, TScalar>, TConfigurationDerived> : public DiffManifold<typename EuclideanSpaceConf<IDimension, TScalar>::ParentConf, TConfigurationDerived> {
	public:
		typedef DiffManifold<typename EuclideanSpaceConf<IDimension, TScalar>::ParentConf, TConfigurationDerived> parent_t;
		typedef TConfigurationDerived configuration_t;
		typedef internal::DiffManifoldConfigurationTypeTrait<configuration_t> Types;
		typedef typename Types::scalar_t scalar_t;
		typedef typename Types::point_t point_t;
		typedef typename Types::tangent_vector_t tangent_vector_t;
		typedef typename Types::dmatrix_t dmatrix_t;
		typedef typename Eigen::Matrix<scalar_t, configuration_t::Dimension::VALUE, configuration_t::PointSize::VALUE> dlog_matrix_t;

		DiffManifold(configuration_t configuration) : parent_t(configuration) {}
		DiffManifold(int dimension = configuration_t::Dimension::VALUE) : parent_t(configuration_t(dimension)) {}

		void getIdentityInto(point_t & pt) const;
		void randomizePoint(point_t & pt) const;

		static void multInto(const point_t & a, const point_t & b, point_t & result);
		static void logInto(const point_t & from, const point_t & to, tangent_vector_t & result);
		static void expInto(const point_t & point, const tangent_vector_t & vec, point_t & result);
		static void expAtIdInto(const tangent_vector_t & vec, point_t & result);
		void dexpAtIdInto(const tangent_vector_t & vec, dmatrix_t & result) const;
		void dexpInto(const point_t & from, const tangent_vector_t & vec, dmatrix_t & result) const;
	};
}

#include "implementation/EuclideanSpaceImpl.hpp"
#endif /* FLATRIEMANNIANGEOMETRY_HPP_ */
