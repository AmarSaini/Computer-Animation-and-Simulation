
# include "particle_system.h"
# include <sig/gs_string.h>

# include <sigogl/ui_dialogs.h>

//# define GS_USE_TRACE1  // keyword tracing
# include <sig/gs_trace.h>
# include <iostream>

using namespace std;

//======================================== ParticleSystem ===============================================

ParticleSystem::ParticleSystem ()
{
	_time = 0;
	_bounce = true;
	_collides = _collidev = false;
	_gravity.set ( 0, -9.807f, 0 );
	_restitution = 0.5f;
	_colradius = 1.0f;
	_colks=1.0f;
	_colkd=1.0f;

	// Custom
	_method = false;
	_stiffness = 1.0f;
	_impactStrength = 1.0f;

	_benchmarking = false;
	_minPointOfNet = 0.0f;

}

ParticleSystem::~ParticleSystem ()
{
}

void ParticleSystem::init ( const char* cfgfile )
{
	GsInput in;
	if ( !cfgfile ) cfgfile = ui_select_file ( "Enter Configuration File", 0, "*.txt" );
	if ( !cfgfile ) gsout.fatal ( "Configuration file needed!" );
	if ( !in.open(cfgfile) ) gsout.fatal ( "Could not open cfg file!");
	_lcfgfile = cfgfile;

	GsVars* params = new GsVars;
	in.commentchar ( '#' );
	in.lowercase ( false );
	while ( true )
	{	if ( in.get()==GsInput::End ) break;
		GS_TRACE1 ( "Token: "<<in.ltoken() );

		if ( in.ltoken()=="parameters" )
		{	in >> *params;
		}
		else if ( in.ltoken()=="end" )
		{	break;
		}
	}
	GS_TRACE1("End Parsing");

	//init ( params );
	init_cloth(params);
	delete params;
}

void ParticleSystem::init ( const GsVars* vars )
{
	int size = vars->geti("particles",0);

	float mass1 = vars->getf("part.mass",0);
	float mass2 = vars->getf("part.mass",1);

	float rest1 = vars->getf("part.restit",0); if(rest1<=0.1f) rest1=0.1f;
	float rest2 = vars->getf("part.restit",1); if(rest2>=1.0f) rest2=1.0f;

	float vel1 = vars->getf("part.vel",0);
	float vel2 = vars->getf("part.vel",1);
	GsVec vel ( vars->getf("part.dir",0), vars->getf("part.dir",1), vars->getf("part.dir",2) );
	GsString velrand = vars->gets("part.dir.rand");

	_colks = vars->getf("col.ks");
	_colkd = vars->getf("col.kd");

	_restitution = vars->getf("restitution");

	GsVar* v;
	v = vars->get("worldbox");
	_world.set ( GsVec(v->getf(0),v->getf(1),v->getf(2)), GsVec(v->getf(3),v->getf(4),v->getf(5)) );

	v = vars->get("initpos");
	GsPnt posa ( GsVec(v->getf(0),v->getf(1),v->getf(2)) );
	GsPnt posb ( GsVec(v->getf(3),v->getf(4),v->getf(5)) );

	_particles.size ( size );
	_maxmass = 0;
	_time = 0;

	int i;
	GsQuat q;
	GsVec pos;
	for ( i=0; i<size; i++ )
    {	pos.set ( gs_random(posa.x,posb.x), gs_random(posa.y,posb.y), gs_random(posa.z,posb.z) );
		_particles[i].init ( gs_random(mass1,mass2), pos );

		_particles[i].r = gs_random(rest1,rest2);

		if ( _particles[i].m>_maxmass ) _maxmass=_particles[i].m;

		vel.len ( gs_random(vel1,vel2) );
		_particles[i].v = vel;

		if ( velrand=="random" )
		{	GsQuat q;
			q.random();
			_particles[i].v = q.apply(vel);
		}
	}
}

void ParticleSystem::get_state ()
{
	_state.size ( _particles.size()*6 );
	float* s = &_state[0];
	for ( int i=0, size=_particles.size(); i<size; i++ )
    {	Particle& p = _particles[i];
		*(s++)=p.x.x; *(s++)=p.x.y; *(s++)=p.x.z;
		*(s++)=p.v.x; *(s++)=p.v.y; *(s++)=p.v.z;
	}
}

void ParticleSystem::set_state ()
{
	const float* s = &_state[0];
	int n = int(sqrt(_particles.size()));
	for ( int i=0, size=_particles.size(); i<size; i++ )
	{	Particle& p = _particles[i];
		p.x.x=*(s++); p.x.y=*(s++); p.x.z=*(s++);
		p.v.x=*(s++); p.v.y=*(s++); p.v.z=*(s++);

		if (_benchmarking && p.x.y < _minPointOfNet && i < n*n) {
			_minPointOfNet = p.x.y;
			cout << "New Minimum: " << p.x.y << endl;
		}

	}
}

void ParticleSystem::compute_derivatives ()
{
	clear_forces ();
	compute_forces ();

	_derivatives.size ( _particles.size()*6 );
	float* s = &_derivatives[0];

	for ( int i=0, size=_particles.size(); i<size; i++ )
	{	Particle& p = _particles[i];
		*(s++) = p.v.x;
		*(s++) = p.v.y;
		*(s++) = p.v.z;
		*(s++) = p.f.x/p.m;
		*(s++) = p.f.y/p.m;
		*(s++) = p.f.z/p.m;
	}
}

void ParticleSystem::clear_forces ()
{
	for ( int i=0, size=_particles.size(); i<size; i++ )
	{	_particles[i].f = GsVec::null;
	}
}

void ParticleSystem::compute_forces ()
{
	int i, j, size=_particles.size();

	for ( i=0; i<size; i++ )
	{	_particles[i].f += (_particles[i].m*_gravity) + _extforce;
	}

	if ( _collides )
	{
		float dist, r2=_colradius*2.0f; 
		GsVec l, dv, f;

		for ( i=0; i<size; i++ )
		{
			for ( j=0; j<size; j++ )
			{	if ( i==j) continue;
				dist = ::dist(_particles[i].x,_particles[j].x);
				if ( dist>0.0001f && dist<r2 )
				{	l = _particles[i].x-_particles[j].x;
					dv = _particles[i].v-_particles[j].v;
					f = l / dist * -( _colks*(dist-r2) + dot(dv,l)*_colkd/dist); // spring behavior
					_particles[i].f += f;

					/*GsVec n = l; n.normalize();
					f = _particles[i].f;
					_particles[i].f += -f*dot(n,f);*/
				}
			}
		}
	}

	if ( 0 )
	{	GsVec n = GsVec::j; // bottom plane normal pointing up
		GsVec v, nv;
		for ( i=0; i<size; i++ )
		{	// we do something simple here with the "floor":
			if ( _particles[i].x.y<_world.a.y  &&  dot(_particles[i].v,n)<0 )
			{	//_particles[i].v = reflect(n,_particles[i].v) * _restitution * _particles[i].r;
				GsVec f = _particles[i].f;
				_particles[i].f += f * -dot(f,n);
			}
		}
	}

}

inline GsVec reflect ( const GsVec& n, const GsVec& v )
{
	GsVec nv, tv;
	nv = n * dot ( v, n ); // normal component
	tv = v - nv;           // tangential component
	return tv-nv;
}

void ParticleSystem::euler_step ( float deltat )
{
	compute_derivatives ();
	int i, stsize=_derivatives.size();

	for ( i=0; i<stsize; i++ ) _derivatives[i]*=deltat;
	get_state ();
	for ( i=0; i<stsize; i++ ) _state[i]+=_derivatives[i];

	set_state ();

	_time += deltat;

	if ( _collidev )
	{	float dist, r2=_colradius*2.0f; 
		GsVec n, v;
		int i, j, psize=_particles.size();

		for ( i=0; i<psize; i++ )
		{ 
			for ( j=0; j<psize; j++ )
			{	if ( i==j) continue;
				dist = ::dist(_particles[i].x,_particles[j].x);
				if ( dist>0.0001f && dist<r2 )
				{	n = _particles[i].x-_particles[j].x;
					n.normalize();
					v=_particles[i].v;
					v.normalize();
					if ( dot(n,v)<0 )
						_particles[i].v = reflect(n,_particles[i].v) * _restitution * _particles[i].r;
				}
			}
		}
	}

	// to try: bounce against all planes!
	if ( _bounce )
	{	GsVec n = GsVec::j; // bottom plane normal pointing up
		GsVec v, nv;
		for ( i=0; i<_particles.size(); i++ )
		{	// we do something simple here with the "floor":
			if ( _particles[i].x.y<_world.a.y  &&  dot(_particles[i].v,n)<0 )
			{	_particles[i].v = reflect(n,_particles[i].v) * _restitution * _particles[i].r;
			}
		}
	}

}


// Custom

void ParticleSystem::init_cloth(const GsVars* vars)
{
	int size = vars->geti("particles", 0);

	float mass1 = vars->getf("part.mass", 0);
	float mass2 = vars->getf("part.mass", 1);

	float rest1 = vars->getf("part.restit", 0); if (rest1 <= 0.1f) rest1 = 0.1f;
	float rest2 = vars->getf("part.restit", 1); if (rest2 >= 1.0f) rest2 = 1.0f;

	float vel1 = vars->getf("part.vel", 0);
	float vel2 = vars->getf("part.vel", 1);
	GsVec vel(vars->getf("part.dir", 0), vars->getf("part.dir", 1), vars->getf("part.dir", 2));
	GsString velrand = vars->gets("part.dir.rand");

	_colks = vars->getf("col.ks");
	_colkd = vars->getf("col.kd");

	_restitution = vars->getf("restitution");

	GsVar* v;
	v = vars->get("worldbox");
	_world.set(GsVec(v->getf(0), v->getf(1), v->getf(2)), GsVec(v->getf(3), v->getf(4), v->getf(5)));

	v = vars->get("initpos");
	GsPnt posa(GsVec(v->getf(0), v->getf(1), v->getf(2)));
	GsPnt posb(GsVec(v->getf(3), v->getf(4), v->getf(5)));

	_particles.size(size);
	_maxmass = 0;
	_time = 0;

	int i;
	GsQuat q;
	GsVec pos;

	// Row for cloths. Every 10 particles go down a row.
	int j = -1;

	// NxN particle cloth
	int n = int(sqrt(size));
	for (i = 0; i < n * n; i++)
	{
		if ((i % n) == 0) {
			j++;
		}

		pos.set(float(-n + 2*(i%n)), float(0), float(n - 2 * j));

		_particles[i].init(gs_random(1.0f, 1.0f), pos);

		_particles[i].r = gs_random(rest1, rest2);

		if (_particles[i].m>_maxmass) _maxmass = _particles[i].m;

		vel.len(gs_random(vel1, vel2));
		_particles[i].v = vel;

		if (velrand == "random")
		{
			GsQuat q;
			q.random();
			_particles[i].v = q.apply(vel);
		}
	}

	// Falling Particles

	for (i = n * n; i < size; i++) {
		pos.set(gs_random(posa.x, posb.x), gs_random(posa.y, posb.y), gs_random(posa.z, posb.z));
		_particles[i].init(gs_random(5.0f, 5.0f), pos);

		_particles[i].r = gs_random(rest1, rest2);

		if (_particles[i].m>_maxmass) _maxmass = _particles[i].m;

		vel.len(gs_random(vel1, vel2));
		_particles[i].v = vel;

		if (velrand == "random")
		{
			GsQuat q;
			q.random();
			_particles[i].v = q.apply(vel);
		}
	}

}

void ParticleSystem::get_state_verlet(float deltat)
{
	_state.size(_particles.size() * 6);
	float* s = &_state[0];
	for (int i = 0, size = _particles.size(); i<size; i++)
	{
		Particle& p = _particles[i];
		*(s++) = p.x.x; *(s++) = p.x.y; *(s++) = p.x.z;
		*(s++) = p.x.x - (p.v.x * deltat); *(s++) = p.x.y - (p.v.y * deltat); *(s++) = p.x.z - (p.v.z * deltat);
	}
}

void ParticleSystem::set_state_verlet(float deltat)
{
	const float* s = &_state[0];
	int n = int(sqrt(_particles.size()));
	for (int i = 0, size = _particles.size(); i<size; i++)
	{
		Particle& p = _particles[i];
		p.x.x = *(s++); p.x.y = *(s++); p.x.z = *(s++);
		p.v.x = (p.x.x - *(s++))/deltat; p.v.y = (p.x.y - *(s++))/deltat; p.v.z = (p.x.z - *(s++))/deltat;

		if (_benchmarking && p.x.y < _minPointOfNet && i < n*n) {
			_minPointOfNet = p.x.y;
			cout << "New Minimum: " << p.x.y << endl;
		}

	}
}

void ParticleSystem::compute_derivatives_cloth()
{
	clear_forces();
	compute_forces_cloth();
	compute_forces_falling_particles();

	_derivatives.size(_particles.size() * 6);
	float* s = &_derivatives[0];

	for (int i = 0, size = _particles.size(); i<size; i++)
	{
		Particle& p = _particles[i];
		*(s++) = p.v.x;
		*(s++) = p.v.y;
		*(s++) = p.v.z;
		*(s++) = p.f.x / p.m;
		*(s++) = p.f.y / p.m;
		*(s++) = p.f.z / p.m;
	}
}

void ParticleSystem::spring_connect(int i, int j) {

	float dist, r2 = _colradius * 2.0f;
	GsVec l, dv, f;

	dist = ::dist(_particles[i].x, _particles[j].x);

	l = _particles[i].x - _particles[j].x;
	dv = _particles[i].v - _particles[j].v;
	f = l / dist * -(_colks*(dist - r2) + dot(dv, l)*_colkd / dist); // spring behavior
	_particles[i].f += f;

}

void ParticleSystem::compute_forces_cloth()
{
	int size = _particles.size();
	int n = int(sqrt(size));

	// Gravity on all cloth particles

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			_particles[i*n + j].f += (_particles[i*n + j].m*_gravity)/10 + _extforce;
		}
	}

	if (_collides)
	{

		// Horizontal Springs
		
		for (int i = 0; i < n; i++) {
			
			for (int j = 0; j < n-1; j++) {

				spring_connect(i*n + j, i*n + j + 1);
				spring_connect(i*n + j + 1, i*n + j);

			}

		}

		// Vertical Springs
		
		for (int i = 0; i < n-1; i++) {

			for (int j = 0; j < n; j++) {

				spring_connect(i*n + j, (i+1)*n + j);
				spring_connect((i + 1)*n + j, i*n + j);

			}

		}

		// Top-Left to Bottom-Right Diagonal Springs
		
		for (int i = 0; i < n - 1; i++) {

			for (int j = 0; j < n - 1; j++) {

				spring_connect(i*n + j, (i+1)*n + j + 1);
				spring_connect((i + 1)*n + j + 1, i*n + j);

			}

		}

		// Top-Right to Bottom-Left Diagonal Springs
		
		for (int i = 0; i < n - 1; i++) {

			for (int j = 1; j < n; j++) {

				spring_connect(i*n + j, (i + 1)*n + j - 1);
				spring_connect((i + 1)*n + j - 1, i*n + j);

			}

		}
		
		// Loose Anchors.
		/*
		_particles[0].f = GsVec::null;
		_particles[n-1].f = GsVec::null;
		_particles[n*n - n].f = GsVec::null;
		_particles[n*n - 1].f = GsVec::null;
		*/

		// Stiff Anchors.
		for (int i = 0; i < n; i++) {
			_particles[i].f = GsVec::null;
			_particles[i*n].f = GsVec::null;
			_particles[i*n + n-1].f = GsVec::null;
			_particles[n*(n-1) + i].f = GsVec::null;
		}
		
	}

}

void ParticleSystem::compute_forces_falling_particles() {

	int size = _particles.size();
	int n = int(sqrt(size));

	float dist, r2 = 4.0f;
	GsVec l, dv, f;

	// Gravity for falling particles

	for (int i = n * n; i < size; i++) {
		_particles[i].f += (_particles[i].m*_gravity) + _extforce;
	}

	if (_collides)
	{
		// Check collision against all spring particles, besides stiff anchor points.
		for (int k = n * n; k < size; k++) {
			for (int i = 1; i < n-1; i++) {
				for (int j = 1; j < n-1; j++) {
					dist = ::dist(_particles[k].x, _particles[i*n + j].x);
					if (dist > 0.0001f && dist < r2) {
						l = _particles[k].x - _particles[i*n + j].x;
						dv = _particles[k].v - _particles[i*n + j].v;
						f = l / dist * -(_colks*(dist - r2) + dot(dv, l)*_colkd / dist); // spring behavior
						_particles[k].f += f;

						l = _particles[i*n + j].x - _particles[k].x;
						dv = _particles[i*n + j].v - _particles[k].v;
						f = l / dist * -(_colks*_impactStrength*(dist - r2) + dot(dv, l)*_colkd*_impactStrength / dist); // spring behavior
						_particles[i*n + j].f += f;

					}
				}
			}
		}

		float dist, r2 = 6.0f;
		GsVec l, dv, f;

		// Check collision against other falling particles.
		for (int i = n * n; i < size; i++) {
			for (int j = n * n; j < size; j++) {
				if (i == j) continue;
				dist = ::dist(_particles[i].x, _particles[j].x);
				if (dist>0.0001f && dist<r2)
				{
					l = _particles[i].x - _particles[j].x;
					dv = _particles[i].v - _particles[j].v;
					f = l / dist * -(_colks*(dist - r2) + dot(dv, l)*_colkd / dist); // spring behavior
					_particles[i].f += f;

					/*GsVec n = l; n.normalize();
					f = _particles[i].f;
					_particles[i].f += -f*dot(n,f);*/
				}
			}
		}

	}

}

void ParticleSystem::integration_step(float deltat) {

	if (!_method) {
		euler_step_cloth(deltat);
	}

	else {
		verlet_step_cloth(deltat);
	}

}

void ParticleSystem::euler_step_cloth(float deltat)
{
	compute_derivatives_cloth();
	int i, stsize = _derivatives.size();

	for (i = 0; i<stsize; i++) _derivatives[i] *= deltat;

	get_state();

	for (i = 0; i<stsize; i++) _state[i] += _derivatives[i];

	set_state();

	_time += deltat;

}

void ParticleSystem::verlet_step_cloth(float deltat)
{
	compute_derivatives_cloth();
	int i, stsize = _derivatives.size();
	bool geometric_constraint = true;


	get_state_verlet(deltat);

	if (!geometric_constraint) {

		for (i = 0; i < stsize; i += 6) {
			float oldPosX = _state[i + 0];
			float oldPosY = _state[i + 1];
			float oldPosZ = _state[i + 2];
			_state[i + 0] = 2 * _state[i + 0] - _state[i + 3] + _derivatives[i + 3] * deltat * deltat;
			_state[i + 1] = 2 * _state[i + 1] - _state[i + 4] + _derivatives[i + 4] * deltat * deltat;
			_state[i + 2] = 2 * _state[i + 2] - _state[i + 5] + _derivatives[i + 5] * deltat * deltat;
			_state[i + 3] = oldPosX;
			_state[i + 4] = oldPosY;
			_state[i + 5] = oldPosZ;
		}

	}

	else {

		for (i = 0; i < stsize; i += 6) {
			float oldPosX = _state[i + 0];
			float oldPosY = _state[i + 1];
			float oldPosZ = _state[i + 2];
			_state[i + 0] = 2 * _state[i + 0] - _state[i + 3] + _derivatives[i + 3] * deltat * deltat;
			_state[i + 1] = 2 * _state[i + 1] - _state[i + 4] + _derivatives[i + 4] * deltat * deltat;
			_state[i + 2] = 2 * _state[i + 2] - _state[i + 5] + _derivatives[i + 5] * deltat * deltat;
			_state[i + 3] = oldPosX;
			_state[i + 4] = oldPosY;
			_state[i + 5] = oldPosZ;

			if (::dist(GsVec(_state[i + 0], _state[i + 1], _state[i + 2]), GsVec(_state[i + 3], _state[i + 4], _state[i + 5])) > 0.02) {

				_state[i + 0] = _state[i + 3] + (_state[i + 0] - _state[i + 3]) / _stiffness;
				_state[i + 1] = _state[i + 4] + (_state[i + 1] - _state[i + 4]) / _stiffness;
				_state[i + 2] = _state[i + 5] + (_state[i + 2] - _state[i + 5]) / _stiffness;

			}

		}

	}

	set_state_verlet(deltat);

	_time += deltat;

}

//======================================== EOF ===============================================
