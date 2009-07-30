// 2009 Geoyeob Kim
// Type definitions for cml template classes.
///
#pragma once

typedef cml::quaternion< float, cml::fixed<>, cml::scalar_first, cml::positive_cross > cml_quat;
typedef cml::vector< float, cml::fixed<3> > cml_vec3;
typedef cml::matrix33f_c cml_mat33;
typedef cml::matrix44f_c cml_mat44;
