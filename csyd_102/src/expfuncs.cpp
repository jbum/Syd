#include "ss.h"
#include "expmgr.h"
#include <math.h>

// #include "MainWin.h"

// Computes discrete potential levels around mandelbrot set.
// Outputs: 0 = inside mandelbrot set
//          1 -> n potential level

int	ExpMgr::CalcMandel(ssfloat p0, ssfloat q0)
{
	// Optimization for multiple calculations on the same pixel
	if (p0 == lastMandelX && q0 == lastMandelY)
		return lastMandelVal;
	else {
		ssfloat	x=myNIL,y=myNIL,x1,y1;
		int	kol=0,limit;

		limit = recur;

		x1 = y1 = x;
		do {
			x1 = x1 - y1 + p0;
			y1 = 2*x*y + q0;
			++kol;
			x = x1;
			y = y1;
			x1 *= x1;
			y1 *= y1;
		} while (kol <= limit && x1+y1 < 4.0);

		if (kol > limit)
			kol = 0;
		lastMandelX = p0;
		lastMandelY = q0;
		lastMandelVal = kol;
		return kol;
	}
}

ssfloat	ExpMgr::CalcMandelCPM(ssfloat p0, ssfloat q0)
{
	// Optimization for multiple calculations on the same pixel
	if (p0 == lastMandelX && q0 == lastMandelY)
		return lastMandelCPM;
	else {
		ssfloat	x=myNIL,y=myNIL,x1,y1,retVal,sumxy;
		int	kol=0,limit;

		limit = recur;


		x1 = x*x;
		y1 = y*y;
		kol = 0;
		do {
			x1 = x1-y1+p0;
			y = 2*x*y+q0;
			x = x1;
			y1 = y;
			x1 *= x1;
			y1 *= y1;
			++kol;
		} while (kol <= limit && x1+y1 < 10000);
		sumxy = x1+y1;
		if (kol > limit)
			retVal = 0.0;
		else
			retVal = -log(log10(sumxy) / pow(2,kol));


		lastMandelX = p0;
		lastMandelY = q0;
		lastMandelCPM = retVal;
		return retVal;
	}
}

int	ExpMgr::CalcDragon(ssfloat x0, ssfloat y0, ssfloat p0, ssfloat q0)		// z = kz*(1-z)
{
	// Optimization for multiple calculations on the same pixel
	if (p0 == lastMandelX && q0 == lastMandelY)
		return lastMandelVal;
	else {
		ssfloat	x=myNIL,y=myNIL,x1,y1;
		int	kol=0,limit;

		limit = recur;

		x1 = y1 = x;
		x = p0;
		y = q0;
		do {
			x1 = x*x0 - y*y0;
			y1 = y*x0 + x*y0;
			x = x1 - x*x1 + y*y1;
			y = y1 - x*y1 + y*y1 - y*x1;
			++kol;
		} while (kol <= limit && x+y < 4.0);

		if (kol > limit)
			kol = 0;
		lastMandelX = p0;
		lastMandelY = q0;
		lastMandelVal = kol;
		return kol;
	}
}

int	ExpMgr::CalcMandel3(ssfloat p0, ssfloat q0)
{
	ssfloat	x=myNIL,y=myNIL,x1,y1,x2,y2;
	int	kol=0,limit;

	limit = recur;

	// Optimization for multiple calculations on the same pixel
	if (p0 == lastMandelX && q0 == lastMandelY)
		return lastMandelVal;

	x2 = x*x;
	y2 = y*y;
	do {
		x1 = x2*x - 3*x*y2 + p0;
		y1 = 3*x2*y - y2*y + q0;
		++kol;
		x = x1;
		y = y1;
		x2 = x*x;
		y2 = y*y;
	} while (kol <= limit && x2+y2 < 4.0);

	if (kol > limit)
		kol = 0;
	lastMandelX = p0;
	lastMandelY = q0;
	lastMandelVal = kol;
	return kol;
}


/***
Q7a: What is the difference between the Mandelbrot set and a Julia set?
A7a: The Mandelbrot set iterates z^2+c with z starting at 0 and varying c.
The Julia set iterates z^2+c for fixed c and varying starting z values. That
is, the Mandelbrot set is in parameter space (c-plane) while the Julia set is
in dynamical or variable space (z-plane).
***/

// Here, Z is represented by x,y and c is represented by p,q

int	ExpMgr::CalcJulia(ssfloat x, ssfloat y, ssfloat p0, ssfloat q0)
{
	// Optimization for multiple calculations on the same pixel
	if (x == lastMandelX && y == lastMandelY)
		return lastMandelVal;
	else {
		ssfloat	x1,y1;
		int	kol=0,limit;

		limit = recur;

		lastMandelX = x;
		lastMandelY = y;

		x1 = x*x;
		y1 = y*y;
		do {
			x1 = x1 - y1 + p0;
			y1 = 2*x*y + q0;
			++kol;
			x = x1;
			y = y1;
			x1 *= x1;
			y1 *= y1;
		} while (kol <= limit && x1+y1 < 4.0);

		if (kol > limit)
			kol = 0;
		lastMandelVal = kol;
		return kol;
	}
}

// A random number corresponding to a point on an integer matrix
// Returns 0 <= r < 1

ssfloat ExpMgr::RandPoint(short x, short y)
{
	long	sr;
	ssfloat	r;
	if (x < 0)
		x = -x;
	if (y < 0)
		y = -y;
	sr = GetSRand();
	MySRand(yA[y%256] ^ xA[x%256]);
	r = DoubleRandom();
	MySRand(sr);
	return r;
}

ssfloat ExpMgr::RandPoint3D(short x, short y, short z)
{
	long	sr;
	ssfloat	r;
	if (x < 0)
		x = -x;
	if (y < 0)
		y = -y;
	if (z < 0)
		z = -z;
	sr = GetSRand();
	MySRand(xA[x%256] ^ yA[y%256] ^ zA[z%256]);
	r = DoubleRandom();
	MySRand(sr);
	return r;
}


// A random number corresponding to a point in a continuous field
// Returns 0 <= r < 1

ssfloat ExpMgr::Noise(ssfloat x, ssfloat y)
{
	ssfloat	dx,dy,x1,x2;
	int		ix,iy;

	if (x < 0)
		x = -x;

	if (y < 0)
		y = -y;

	ix = (int) TruncXToLong(x);
	iy = (int) TruncXToLong(y);
	dx = x - ix;
	dy = y - iy;

	// To avoid linear interpolation artifacts due to sharp corners, we round
	// the edges (ease-in and ease-out)
	dx *= dx*(3-2*dx);
	dy *= dy*(3-2*dy);

	x1 = (1-dx)*RandPoint(ix,iy)   + dx*RandPoint(ix+1,iy);
	x2 = (1-dx)*RandPoint(ix,iy+1) + dx*RandPoint(ix+1,iy+1);
	return (1-dy)*x1 + dy*x2;
}

ssfloat ExpMgr::Noise3D(ssfloat x, ssfloat y, ssfloat z)
{
	ssfloat	dx,dy,dz,j1,j2,j3,k1,k2,k3;
	int		ix,iy,iz;

	if (x < 0)
		x = -x;

	if (y < 0)
		y = -y;

	if (z < 0)
		z = -z;

	ix = (int) TruncXToLong(x);
	iy = (int) TruncXToLong(y);
	iz = (int) TruncXToLong(z);
	dx = x - ix;
	dy = y - iy;
	dz = z - iz;

	// To avoid linear interpolation artifacts due to sharp corners, we round
	// the edges (ease-in and ease-out)
	dx *= dx*(3-2*dx);
	dy *= dy*(3-2*dy);
	dz *= dz*(3-2*dz);

	j1 = (1-dx)*RandPoint3D(ix,iy,iz)   + dx*RandPoint3D(ix+1,iy,iz);
	j2 = (1-dx)*RandPoint3D(ix,iy+1,iz) + dx*RandPoint3D(ix+1,iy+1,iz);
	j3 = (1-dy)*j1 + dy*j2;

	if (dz == 0.0)
		return j3;
	else {
		k1 = (1-dx)*RandPoint3D(ix,iy,iz+1)   + dx*RandPoint3D(ix+1,iy,iz+1);
		k2 = (1-dx)*RandPoint3D(ix,iy+1,iz+1) + dx*RandPoint3D(ix+1,iy+1,iz+1);
		k3 = (1-dy)*k1 + dy*k2;
		return (1-dz)*j3 + dz*k3;
	}
}

// Fractal Disturbance
// Returns -1 <= r < 1

ssfloat ExpMgr::Turbulence(ssfloat x, ssfloat y)
{
	ssfloat	turb, s, limit;

	turb = -1.0;
	s = 1;
	limit = 1.0/256;

	while (s > limit) {
		turb += s * Noise(x/s,y/s);
		s /= 2;
	}

	return turb;
}

ssfloat ExpMgr::Turbulence3D(ssfloat x, ssfloat y, ssfloat z)
{
	ssfloat	turb, s, limit;

	turb = -1.0;
	s = 1;
	limit = 1.0/256;

	while (s > limit) {
		turb += s * Noise3D(x/s,y/s,z/s);
		s /= 2;
	}

	return turb;
}

ssfloat ExpMgr::GTurbulence3D(ssfloat x, ssfloat y, ssfloat z)
{
	ssfloat	turb, s, limit;

	turb = 0;
	s = 1;
	limit = 1.0/256;

	while (s > limit) {
		turb += s * gnoise(x/s,y/s,z/s);
		s /= 2;
	}

	return turb / 2;
}

#define snoise(x,h)		(2*Noise(x,y)-1)
#define SNoise3D(x,y,z)		(Noise3D(x,y,z)*2-1.0)
#define SMOOTHSTEP(x)  ((x)*(x)*(3 - 2*(x)))
static ssfloat gradientTab[TABSIZE*3];

static void gradientTabInit(int seed);
static ssfloat glattice(int ix, int iy, int iz, ssfloat fx, ssfloat fy, ssfloat fz);


#define    srandom(seed)	MySRand(seed)
#undef	RANDNBR
#define		RANDNBR			MyRandom()

// #define MINFREQ	0.001
// #define MAXFREQ	1

ssfloat ExpMgr::fBm(ssfloat x, ssfloat y, ssfloat z,
		 ssfloat H, ssfloat lacunarity, ssfloat octaves)
{
	ssfloat value, frequency, remainder;
	int		i,iOctaves=(int)octaves;
	static Boolean first=true;
	static ssfloat* exponent_array;
	if (first) {
		exponent_array=(ssfloat *) MyNewPtrClear((iOctaves+1)*sizeof(ssfloat));
		frequency = 1.0;
		for (i = 0; i <= iOctaves; i++) {
			exponent_array[i] = pow( frequency, -H );
			frequency *= lacunarity;
		}
		first = false;
	}
	value = 0.0;
	frequency = 1.0;
	for (i = 0; i < iOctaves; i++) {
		value += SNoise3D(x,y,z) * exponent_array[i];
		x *= lacunarity;
		y *= lacunarity;
		z *= lacunarity;
	}
	remainder = octaves - iOctaves;
	if (remainder)
		value += remainder * SNoise3D(x,y,z) * exponent_array[i];
	return value;
}


ssfloat ExpMgr::gnoise(ssfloat x, ssfloat y, ssfloat z)
{
    int ix, iy, iz;
    ssfloat fx0, fx1, fy0, fy1, fz0, fz1;
    ssfloat wx, wy, wz;
    ssfloat vx0, vx1, vy0, vy1, vz0, vz1;
    static int initialized = 0;

    if (!initialized) {
        gradientTabInit(665);
        initialized = 1;
    }

    ix = FLOOR(x);
    fx0 = x - ix;
    fx1 = fx0 - 1;
    wx = SMOOTHSTEP(fx0);

    iy = FLOOR(y);
    fy0 = y - iy;
    fy1 = fy0 - 1;
    wy = SMOOTHSTEP(fy0);

    iz = FLOOR(z);
    fz0 = z - iz;
    fz1 = fz0 - 1;
    wz = SMOOTHSTEP(fz0);

    vx0 = glattice(ix,iy,iz,fx0,fy0,fz0);
    vx1 = glattice(ix+1,iy,iz,fx1,fy0,fz0);
    vy0 = LERP(wx, vx0, vx1);
    vx0 = glattice(ix,iy+1,iz,fx0,fy1,fz0);
    vx1 = glattice(ix+1,iy+1,iz,fx1,fy1,fz0);
    vy1 = LERP(wx, vx0, vx1);
    vz0 = LERP(wy, vy0, vy1);

    vx0 = glattice(ix,iy,iz+1,fx0,fy0,fz1);
    vx1 = glattice(ix+1,iy,iz+1,fx1,fy0,fz1);
    vy0 = LERP(wx, vx0, vx1);
    vx0 = glattice(ix,iy+1,iz+1,fx0,fy1,fz1);
    vx1 = glattice(ix+1,iy+1,iz+1,fx1,fy1,fz1);
    vy1 = LERP(wx, vx0, vx1);
    vz1 = LERP(wy, vy0, vy1);

    return LERP(wz, vz0, vz1);
}

static void gradientTabInit(int seed)
{
    ssfloat *table = gradientTab;
    ssfloat z, r, theta;
    int i;

    srandom(seed);
    for(i = 0; i < TABSIZE; i++) {
        z = 1. - 2.* DoubleRandom();
        /* r is radius of x,y circle */
        r = sqrt(1 - z*z);
        /* theta is angle in (x,y) */
        theta = 2 * pi * DoubleRandom();
        *table++ = r * cos(theta);
        *table++ = r * sin(theta);
        *table++ = z;
    }
}

static ssfloat glattice(int ix, int iy, int iz,
    ssfloat fx, ssfloat fy, ssfloat fz)
{
    ssfloat *g = &gradientTab[INDEX(ix,iy,iz)*3];
    return g[0]*fx + g[1]*fy + g[2]*fz;
}

unsigned char perm[TABSIZE] = {
        225,155,210,108,175,199,221,144,203,116, 70,213, 69,158, 33,252,
          5, 82,173,133,222,139,174, 27,  9, 71, 90,246, 75,130, 91,191,
        169,138,  2,151,194,235, 81,  7, 25,113,228,159,205,253,134,142,
        248, 65,224,217, 22,121,229, 63, 89,103, 96,104,156, 17,201,129,
         36,  8,165,110,237,117,231, 56,132,211,152, 20,181,111,239,218,
        170,163, 51,172,157, 47, 80,212,176,250, 87, 49, 99,242,136,189,
        162,115, 44, 43,124, 94,150, 16,141,247, 32, 10,198,223,255, 72,
         53,131, 84, 57,220,197, 58, 50,208, 11,241, 28,  3,192, 62,202,
         18,215,153, 24, 76, 41, 15,179, 39, 46, 55,  6,128,167, 23,188,
        106, 34,187,140,164, 73,112,182,244,195,227, 13, 35, 77,196,185,
         26,200,226,119, 31,123,168,125,249, 68,183,230,177,135,160,180,
         12,  1,243,148,102,166, 38,238,251, 37,240,126, 64, 74,161, 40,
        184,149,171,178,101, 66, 29, 59,146, 61,254,107, 42, 86,154,  4,
        236,232,120, 21,233,209, 45, 98,193,114, 78, 19,206, 14,118,127,
         48, 79,147, 85, 30,207,219, 54, 88,234,190,122, 95, 67,143,109,
        137,214,145, 93, 92,100,245,  0,216,186, 60, 83,105, 97,204, 52
};


// Returns angle of vector which intersects xd,yd with x axis
// Returns 0 - 2¹

ssfloat ExpMgr::Angle(ssfloat xd, ssfloat yd)
{
	ssfloat	r;
	r = Distance(xd,yd);
	if (yd > 0)
		return (pi*2)-acos(xd/r);
	else
		return acos(xd/r);
}

// Returns length of line which begins at origin and ends at xd,yd

ssfloat ExpMgr::Distance(ssfloat xd, ssfloat yd)
{
	return sqrt(xd*xd+yd*yd);
}


long ExpMgr::Fibonacci(long x)
{
	long n = 1,n1=0,tmp;
	while (x--) {
		tmp = n1;
		n1 = n;
		n = n + tmp;
	}
	return n;
}

long ExpMgr::isPrime(long n)
{
	long	x,l;
	if (n < 4)
		return 1;
	if (!(n & 1))
		return 0;
	x = 3;
	l = n/x;
	while (x < l) {
		if (x*l == n)
			return 0;
		x += 2;
		l = n/x;
	}
	return 1;
}

ssfloat ExpMgr::cpspch(ssfloat pch)
{
	ssfloat	i,f;
	i = (int) pch;
	f = (pch - i)*100/12.0;
	return c1*pow(2,i+f);
}

ssfloat ExpMgr::cpsoct(ssfloat oct)
{
	return c1*pow(2,oct);
}

ssfloat ExpMgr::pchoct(ssfloat oct)
{
	ssfloat	i,f;
	i = (int) oct;
	f = (oct - i)*12/100.0;
	return i + f;
}

ssfloat ExpMgr::octpch(ssfloat pch)
{
	ssfloat	i,f;
	i = (int) pch;
	f = (pch - i)*100/12.0;
	return i + f;
}

ssfloat ExpMgr::octcps(ssfloat cps)
{
	ssfloat octpow;
	octpow = cps/c1;
	return log(octpow) / log2;	// compute base 2 logarithm of octpow (log(octpow) / log(2))
}

ssfloat ExpMgr::octmidi(ssfloat mp)
{
	return 3.0 + mp / 12.0;
}

ssfloat ExpMgr::cpsmidi(ssfloat mp)
{
	return cpsoct(octmidi(mp));
}

ssfloat ExpMgr::linen(ssfloat t, ssfloat atk, ssfloat dur, ssfloat dcy)
{
	ssfloat	sdcy;
	if (t < 0 || t > dur)
		return 0.0;
	if (dcy > dur)
		dcy = dur;
	if (atk > 0 && t < atk) {
		return t/atk;
	}
	sdcy = dur-dcy;
	if (sdcy < 0)
		sdcy = 0;
	if (dcy > 0 && t >= sdcy) {
		return 1.0 - (t-sdcy)/dcy;
	}
	return 1.0;
}

ssfloat ExpMgr::linenr(ssfloat t, ssfloat atk, ssfloat dcy, ssfloat atdec)
{
	if (t < 0 || t > atk+dcy)
		return 0.0;
	if (atk > 0 && t < atk) {
		return t/atk;
	}
	t -= atk;
	return pow(atdec,t/dcy);
}

ssfloat ExpMgr::limit(ssfloat v, ssfloat low, ssfloat high)
{
	if (high < low) {
		return (high-low)/2;
	}
	if (v < low)
		return low;
	if (v > high)
		return high;
	return v;
}
