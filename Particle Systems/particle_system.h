
# pragma once

# include <sig/gs_array.h>
# include <sig/gs_box.h>
# include <sig/gs_quat.h>
# include <sig/gs_vars.h>

# include "particle.h"

class ParticleSystem
 { private :
	GsArray<Particle> _particles;
	GsArray<float> _state;
	GsArray<float> _derivatives;
	float _time;
	GsVec _extforce;
	GsVec _gravity;
	GsBox _world;
	GsPnt _initpos;
	bool _bounce;
	bool _collides, _collidev;
	float _colradius;
	float _colks;
	float _colkd;
	float _restitution;
	float _maxmass;
	GsCharPt _lcfgfile;

	// Custom
	bool _method;
	float _stiffness;
	float _impactStrength;

	// For Benchmark
	bool _benchmarking;
	float _minPointOfNet;

   public :
	ParticleSystem ();
   ~ParticleSystem ();

	const char* last_cfg_file () const { return _lcfgfile; }

	void init ( const char* cfgfile );
	void init ( const GsVars* vars );

	void bounce ( bool b ) { _bounce=b; }
	void spring_collision_prevention ( bool sc ) { _collides=sc; }
	void velocity_collision_prevention ( bool vc ) { _collidev=vc; }
	void coeff_restitution ( float r ) { _restitution=r; }
	float coeff_restitution () { return _restitution; }

	void external_force ( const GsVec& f ) { _extforce=f; }
	void collision_radius ( float r ) { _colradius=r; }

	const GsArray<Particle>& particles() const { return _particles; }
	const GsBox& world() const { return _world; }

	void get_state ();
	void set_state ();
	void clear_forces ();
	void compute_forces ();
	void compute_derivatives ();

	void euler_step ( float deltat );

	// Custom

	void select_integration_method(bool m) { _method = m; }
	void set_stiffness(float s) { _stiffness = s; }
	void set_impactStrength(float i) { _impactStrength = i; }

	void init_cloth(const GsVars* vars);
	void get_state_verlet(float deltat);
	void set_state_verlet(float deltat);
	void compute_derivatives_cloth();
	void spring_connect(int i, int j);
	void compute_forces_cloth();
	void compute_forces_falling_particles();
	void integration_step(float deltat);
	void euler_step_cloth(float deltat);
	void verlet_step_cloth ( float deltat );
};
