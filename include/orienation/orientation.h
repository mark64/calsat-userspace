/*
 * gyro, accel, magnetometer, and temperature sensor
 * provides a quaternion with attitude data
 *
 * by Mark Hill 2017
 */
#ifndef __orientation_h
#define __orientation_h

#include<stdint.h>
#include<eigen/dense>

using namespace Eigen;

/**
 * NOTE:All 3D vectors have their basis vectors determined by
 * 		the right hand rule
 * 		The top of the device is +z, the left is +y, and the front +x
 */

/**
 * determines the acceleration of the device using its accelerometer
 * given in m/s/s
 *
 * @return			a three dimensional vector representing acceleration
 */
Vector3d acceleration();

/**
 * determines the rotation rate of the device using its gyroscope
 * given in radians per second
 *
 * @return			a three dimensional vector representing rotation
 */
Vector3d angularRotationRate();

/**
 * determines the magnetic field surrounding the device using its magnetometer
 * given in microTelsas
 *
 * @return			a three dimensional vector representing mag field
 */
Vector3d magneticField();

/**
 * determines the position of the device using its gps
 * position is given as latitude, longitude, altitude
 *
 * @return			a three dimensional vector representing position
 */
Vector3d position();



/**
 * determines the filtered and corrected orientation relative
 * 	to a normal vector from earth's surface directly below
 *
 * @return			a quaternion representing orientation
 */
Vector4d orientation();

/**
 * similar to orientation() but relative to a vector pointing
 *	from the sun to earth
 *
 * @return			a quaternion representing orientation
 */
Vector4d orientationRelativeToSun();

#endif
