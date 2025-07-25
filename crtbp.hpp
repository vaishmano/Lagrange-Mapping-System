#pragma once

#include "math.hpp"

/// <summary>
/// Class that contains the analytic model for the circular restricted three body problem.
/// </summary>
class CRTBP
{
public:
	// Sun-Earth mass ratio.
	//static constexpr double mu = 0.00000304042338912411;

	// Earth-Moon mass ratio.
	//static constexpr double mu = 0.012150585609624;

	// Artificial mass ratio.
	static constexpr double mu = 0.02;

	// Normalized rotation rate of the two massive bodies.
	static constexpr double omega = 1.0;

	/// <summary>
	/// Gets the position of the Sun in a steady co-rotating reference frame.
	/// </summary>
	/// <returns>2D position of the Sun.</returns>
	static Vector2d Sun() { return Vector2d(-mu, 0); }

	/// <summary>
	/// Gets the position of the Earth in a steady co-rotating reference frame.
	/// </summary>
	/// <returns>2D position of the Earth.</returns>
	static Vector2d Earth() { return Vector2d(1-mu, 0); }

	/// <summary>
	/// Calculates the acceleration of a third body in a rotating reference frame.
	/// </summary>
	/// <param name="state">State vector of the third body, containing position and velocity.</param>
	/// <param name="omega">Angular velocity of the rotation of the reference frame.</param>
	/// <returns></returns>
	static Vector4d Direction(const Vector4d& state)
	{
		Vector2d pos(state.x(), state.y());
		Vector2d vel(state.z(), state.w());
		Matrix2d omega_cross;
		omega_cross << 0, omega, -omega, 0;
		Vector2d acc = 
			omega * omega * pos			// centrifugal
			+ 2 * omega_cross * vel		// coriolis
			+ Acceleration(pos);		// gravitational
		return Vector4d(vel.x(), vel.y(), acc.x(), acc.y());
	}

	/// <summary>
	/// Samples the pseudo potential at a given location.
	/// </summary>
	/// <param name="pos">Location to sample the pseudo potential at.</param>
	/// <returns>Scalar-valued pseudo potential.</returns>
	static double PseudoPotential(const Vector2d& pos) {
		double x = pos.x(), y = pos.y(), x2 = x * x, y2 = y * y;
		return mu / std::sqrt(y2 + pow(-x - mu + 1, 2)) + (1 - mu) / std::sqrt(y2 + pow(-x - mu, 2)) + (omega * omega * (y2 + x2)) / 2;
	}

	/// <summary>
	/// Samples the spatial gradient of the pseudo potential at a given location.
	/// </summary>
	/// <param name="pos">Location to sample the gradient of the pseudo potential at.</param>
	/// <returns>Vector-valued gradient of the pseudo potential.</returns>
	static Vector2d PseudoPotentialGrad(const Vector2d& pos) { 
		double x = pos.x(), y = pos.y(), x2 = x * x, y2 = y * y;
		return Vector2d(
			(mu * (-x - mu + 1)) / pow(y2 + pow(-x - mu + 1, 2), 1.5) + ((1 - mu) * (-x - mu)) / pow(y2 + pow(-x - mu, 2), 1.5) + omega * omega * x,
			- (mu * y) / pow(y2 + pow(-x - mu + 1, 2), 1.5) - ((1 - mu) * y) / pow(y2 + pow(-x - mu, 2), 1.5) + omega * omega * y
		);
	}

	/// <summary>
	/// Samples the Hessian matrix of the pseudo potential at a given location.
	/// </summary>
	/// <param name="pos">Location to sample the Hessian of the pseudo potential at.</param>
	/// <returns>Matrix-valued Hessian of the pseudo potential.</returns>
	static Matrix2d PseudoPotentialHessian(const Vector2d& pos) {
		double x = pos.x(), y = pos.y(), x2 = x * x, y2 = y * y;
		double hxx = -mu / pow(y2 + pow(-x - mu + 1, 2), 1.5) + (3 * mu * pow(-x - mu + 1, 2)) / pow(y2 + pow(-x - mu + 1, 2), 2.5) - (1 - mu) / pow(y2 + pow(-x - mu, 2), 1.5) + (3 * (1 - mu) * pow(-x - mu, 2)) / pow(y2 + pow(-x - mu, 2), 2.5) + omega * omega;
		double hxy = -(3 * mu * (-x - mu + 1) * y) / pow(y2 + pow(-x - mu + 1, 2), 2.5) - (3 * (1 - mu) * (-x - mu) * y) / pow(y2 + pow(-x - mu, 2), 2.5);
		double hyy = -mu / pow(y2 + pow(-x - mu + 1, 2), 1.5) + (3 * mu * y2) / pow(y2 + pow(-x - mu + 1, 2), 2.5) - (1 - mu) / pow(y2 + pow(-x - mu, 2), 1.5) + (3 * (1 - mu) * y2) / pow(y2 + pow(-x - mu, 2), 2.5) + omega * omega;
		Matrix2d hessian;
		hessian << hxx, hxy, hxy, hyy;
		return hessian;
	}

	/// <summary>
	/// Samples the Jacobi constant at a given location for a certain velocity magnitude.
	/// </summary>
	/// <param name="pos">Position to sample the Jacobi constant at.</param>
	/// <param name="v0">Velocity magnitude to compute the Jacobi constant at.</param>
	/// <returns>Scalar-valued Jacobi constant.</returns>
	static double JacobiConstant(const Vector2d& pos, const double& v0) { return 2 * PseudoPotential(pos) - v0; }

private:

	/// <summary>
	/// Calculates the acceleration of a third body in a steady co-rotating reference frame.
	/// </summary>
	/// <param name="pos">Position of the third body.</param>
	/// <returns>Acceleration in the gravitational field.</returns>
	static Vector2d Acceleration(const Vector2d& pos)
	{
		Vector2d sun_dir = Sun() - pos;
		Vector2d earth_dir = Earth() - pos;
		return (1 - mu) / std::pow(sun_dir.norm(), 3) * sun_dir
			+ (mu) / std::pow(earth_dir.norm(), 3) * earth_dir;
	}
};