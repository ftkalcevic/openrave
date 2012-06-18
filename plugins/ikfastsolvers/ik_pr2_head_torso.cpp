#define IKFAST_NAMESPACE ik_pr2_head_torso
#include "plugindefs.h"

/// autogenerated analytical inverse kinematics code from ikfast program part of OpenRAVE
/// \author Rosen Diankov
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///     http://www.apache.org/licenses/LICENSE-2.0
/// 
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// ikfast version 58 generated on 2012-06-18 23:21:21.594521
/// To compile with gcc:
///     gcc -lstdc++ ik.cpp
/// To compile without any main function as a shared object (might need -llapack):
///     gcc -fPIC -lstdc++ -DIKFAST_NO_MAIN -DIKFAST_CLIBRARY -shared -Wl,-soname,libik.so -o libik.so ik.cpp
#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>
#include <complex>

#ifdef IKFAST_HEADER
#include IKFAST_HEADER
#endif

#ifndef IKFAST_ASSERT
#include <stdexcept>
#include <sstream>
#include <iostream>

#ifdef _MSC_VER
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCDNAME__
#endif
#endif

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __func__
#endif

#define IKFAST_ASSERT(b) { if( !(b) ) { std::stringstream ss; ss << "ikfast exception: " << __FILE__ << ":" << __LINE__ << ": " <<__PRETTY_FUNCTION__ << ": Assertion '" << #b << "' failed"; throw std::runtime_error(ss.str()); } }

#endif

#if defined(_MSC_VER)
#define IKFAST_ALIGNED16(x) __declspec(align(16)) x
#else
#define IKFAST_ALIGNED16(x) x __attribute((aligned(16)))
#endif

#define IK2PI  ((IKReal)6.28318530717959)
#define IKPI  ((IKReal)3.14159265358979)
#define IKPI_2  ((IKReal)1.57079632679490)

#ifdef _MSC_VER
#ifndef isnan
#define isnan _isnan
#endif
#endif // _MSC_VER

// defined when creating a shared object/dll
#ifdef IKFAST_CLIBRARY
#ifdef _MSC_VER
#define IKFAST_API extern "C" __declspec(dllexport)
#else
#define IKFAST_API extern "C"
#endif
#else
#define IKFAST_API
#endif

// lapack routines
extern "C" {
  void dgetrf_ (const int* m, const int* n, double* a, const int* lda, int* ipiv, int* info);
  void zgetrf_ (const int* m, const int* n, std::complex<double>* a, const int* lda, int* ipiv, int* info);
  void dgetri_(const int* n, const double* a, const int* lda, int* ipiv, double* work, const int* lwork, int* info);
  void dgesv_ (const int* n, const int* nrhs, double* a, const int* lda, int* ipiv, double* b, const int* ldb, int* info);
  void dgetrs_(const char *trans, const int *n, const int *nrhs, double *a, const int *lda, int *ipiv, double *b, const int *ldb, int *info);
  void dgeev_(const char *jobvl, const char *jobvr, const int *n, double *a, const int *lda, double *wr, double *wi,double *vl, const int *ldvl, double *vr, const int *ldvr, double *work, const int *lwork, int *info);
}

using namespace std; // necessary to get std math routines

#ifdef IKFAST_NAMESPACE
namespace IKFAST_NAMESPACE {
#endif

#ifdef IKFAST_REAL
typedef IKFAST_REAL IKReal;
#else
typedef double IKReal;
#endif

class IKSolution
{
public:
    /// Gets a solution given its free parameters
    /// \param pfree The free parameters required, range is in [-pi,pi]
    void GetSolution(IKReal* psolution, const IKReal* pfree) const {
        for(std::size_t i = 0; i < basesol.size(); ++i) {
            if( basesol[i].freeind < 0 )
                psolution[i] = basesol[i].foffset;
            else {
                IKFAST_ASSERT(pfree != NULL);
                psolution[i] = pfree[basesol[i].freeind]*basesol[i].fmul + basesol[i].foffset;
                if( psolution[i] > IKPI ) {
                    psolution[i] -= IK2PI;
                }
                else if( psolution[i] < -IKPI ) {
                    psolution[i] += IK2PI;
                }
            }
        }
    }

    /// Gets the free parameters the solution requires to be set before a full solution can be returned
    /// \return vector of indices indicating the free parameters
    const std::vector<int>& GetFree() const { return vfree; }

    struct VARIABLE
    {
        VARIABLE() : fmul(0), foffset(0), freeind(-1), maxsolutions(1) {
            indices[0] = indices[1] = -1;
        }
        IKReal fmul, foffset; ///< joint value is fmul*sol[freeind]+foffset
        signed char freeind; ///< if >= 0, mimics another joint
        unsigned char maxsolutions; ///< max possible indices, 0 if controlled by free index or a free joint itself
        unsigned char indices[2]; ///< unique index of the solution used to keep track on what part it came from. sometimes a solution can be repeated for different indices. store at least another repeated root
    };

    std::vector<VARIABLE> basesol;       ///< solution and their offsets if joints are mimiced
    std::vector<int> vfree;

    bool Validate() const {
        for(size_t i = 0; i < basesol.size(); ++i) {
            if( basesol[i].maxsolutions == (unsigned char)-1) {
                return false;
            }
            if( basesol[i].maxsolutions > 0 ) {
                if( basesol[i].indices[0] >= basesol[i].maxsolutions ) {
                    return false;
                }
                if( basesol[i].indices[1] != (unsigned char)-1 && basesol[i].indices[1] >= basesol[i].maxsolutions ) {
                    return false;
                }
            }
        }
        return true;
    }

    void GetSolutionIndices(std::vector<unsigned int>& v) const {
        v.resize(0);
        v.push_back(0);
        for(int i = (int)basesol.size()-1; i >= 0; --i) {
            if( basesol[i].maxsolutions != (unsigned char)-1 && basesol[i].maxsolutions > 1 ) {
                for(size_t j = 0; j < v.size(); ++j) {
                    v[j] *= basesol[i].maxsolutions;
                }
                size_t orgsize=v.size();
                if( basesol[i].indices[1] != (unsigned char)-1 ) {
                    for(size_t j = 0; j < orgsize; ++j) {
                        v.push_back(v[j]+basesol[i].indices[1]);
                    }
                }
                if( basesol[i].indices[0] != (unsigned char)-1 ) {
                    for(size_t j = 0; j < orgsize; ++j) {
                        v[j] += basesol[i].indices[0];
                    }
                }
            }
        }
    }
};

inline float IKabs(float f) { return fabsf(f); }
inline double IKabs(double f) { return fabs(f); }

inline float IKsqr(float f) { return f*f; }
inline double IKsqr(double f) { return f*f; }

inline float IKlog(float f) { return logf(f); }
inline double IKlog(double f) { return log(f); }

// allows asin and acos to exceed 1
#ifndef IKFAST_SINCOS_THRESH
#define IKFAST_SINCOS_THRESH ((IKReal)0.000001)
#endif

// used to check input to atan2 for degenerate cases
#ifndef IKFAST_ATAN2_MAGTHRESH
#define IKFAST_ATAN2_MAGTHRESH ((IKReal)2e-6)
#endif

// minimum distance of separate solutions
#ifndef IKFAST_SOLUTION_THRESH
#define IKFAST_SOLUTION_THRESH ((IKReal)1e-6)
#endif

inline float IKasin(float f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return float(-IKPI_2);
else if( f >= 1 ) return float(IKPI_2);
return asinf(f);
}
inline double IKasin(double f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return -IKPI_2;
else if( f >= 1 ) return IKPI_2;
return asin(f);
}

// return positive value in [0,y)
inline float IKfmod(float x, float y)
{
    while(x < 0) {
        x += y;
    }
    return fmodf(x,y);
}

// return positive value in [0,y)
inline double IKfmod(double x, double y)
{
    while(x < 0) {
        x += y;
    }
    return fmod(x,y);
}

inline float IKacos(float f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return float(IKPI);
else if( f >= 1 ) return float(0);
return acosf(f);
}
inline double IKacos(double f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return IKPI;
else if( f >= 1 ) return 0;
return acos(f);
}
inline float IKsin(float f) { return sinf(f); }
inline double IKsin(double f) { return sin(f); }
inline float IKcos(float f) { return cosf(f); }
inline double IKcos(double f) { return cos(f); }
inline float IKtan(float f) { return tanf(f); }
inline double IKtan(double f) { return tan(f); }
inline float IKsqrt(float f) { if( f <= 0.0f ) return 0.0f; return sqrtf(f); }
inline double IKsqrt(double f) { if( f <= 0.0 ) return 0.0; return sqrt(f); }
inline float IKatan2(float fy, float fx) {
    if( isnan(fy) ) {
        IKFAST_ASSERT(!isnan(fx)); // if both are nan, probably wrong value will be returned
        return float(IKPI_2);
    }
    else if( isnan(fx) ) {
        return 0;
    }
    return atan2f(fy,fx);
}
inline double IKatan2(double fy, double fx) {
    if( isnan(fy) ) {
        IKFAST_ASSERT(!isnan(fx)); // if both are nan, probably wrong value will be returned
        return IKPI_2;
    }
    else if( isnan(fx) ) {
        return 0;
    }
    return atan2(fy,fx);
}

inline float IKsign(float f) {
    if( f > 0 ) {
        return float(1);
    }
    else if( f < 0 ) {
        return float(-1);
    }
    return 0;
}

inline double IKsign(double f) {
    if( f > 0 ) {
        return 1.0;
    }
    else if( f < 0 ) {
        return -1.0;
    }
    return 0;
}

/// solves the forward kinematics equations.
/// \param pfree is an array specifying the free joints of the chain.
IKFAST_API void fk(const IKReal* j, IKReal* eetrans, IKReal* eerot) {
IKReal x0,x1,x2,x3,x4,x5;
x0=IKcos(j[1]);
x1=IKsin(j[1]);
x2=IKsin(j[2]);
x3=IKcos(j[2]);
x4=((0.0232000000000000)*(x3));
x5=((0.0980000000000000)*(x2));
eetrans[0]=((-0.0670700000000000)+(((x0)*(x4)))+(((x0)*(x5)))+(((0.0680000000000000)*(x0)))+(((-0.0300000000000000)*(x1))));
eetrans[1]=((((x1)*(x5)))+(((x1)*(x4)))+(((0.0680000000000000)*(x1)))+(((0.0300000000000000)*(x0))));
eetrans[2]=((1.12112500000000)+(((0.0980000000000000)*(x3)))+(j[0])+(((-0.0232000000000000)*(x2))));
eerot[0]=((x0)*(x3));
eerot[1]=((x1)*(x3));
eerot[2]=((-1.00000000000000)*(x2));
}

IKFAST_API int getNumFreeParameters() { return 1; }
IKFAST_API int* getFreeParameters() { static int freeparams[] = {0}; return freeparams; }
IKFAST_API int getNumJoints() { return 3; }

IKFAST_API int getIKRealSize() { return sizeof(IKReal); }

IKFAST_API int getIKType() { return 0x23000006; }

class IKSolver {
public:
IKReal j13,cj13,sj13,htj13,j14,cj14,sj14,htj14,j12,cj12,sj12,htj12,new_px,px,npx,new_py,py,npy,new_pz,pz,npz,pp;
unsigned char _ij13[2], _nj13,_ij14[2], _nj14,_ij12[2], _nj12;

bool ik(const IKReal* eetrans, const IKReal* eerot, const IKReal* pfree, std::vector<IKSolution>& vsolutions) {
j13=numeric_limits<IKReal>::quiet_NaN(); _ij13[0] = -1; _ij13[1] = -1; _nj13 = -1; j14=numeric_limits<IKReal>::quiet_NaN(); _ij14[0] = -1; _ij14[1] = -1; _nj14 = -1;  _ij12[0] = -1; _ij12[1] = -1; _nj12 = 0; 
for(int dummyiter = 0; dummyiter < 1; ++dummyiter) {
    vsolutions.resize(0); vsolutions.reserve(8);
px = eetrans[0]; py = eetrans[1]; pz = eetrans[2];

j12=pfree[0]; cj12=cos(pfree[0]); sj12=sin(pfree[0]);
new_px=((0.0670700000000000)+(px));
new_py=py;
new_pz=((-1.12112500000000)+(((-1.00000000000000)*(j12)))+(pz));
px = new_px; py = new_py; pz = new_pz;
pp=(((px)*(px))+((py)*(py))+((pz)*(pz)));
{
IKReal dummyeval[1];
dummyeval[0]=((-1.00000000000000)+(((-33.3333333333333)*(py))));
if( IKabs(dummyeval[0]) < 0.0000010000000000  )
{
continue;

} else
{
{
IKReal j13array[2], cj13array[2], sj13array[2];
bool j13valid[2]={false};
_nj13 = 2;
IKReal x6=((IKabs(((-0.0600000000000000)+(((-2.00000000000000)*(py))))) != 0)?((IKReal)1/(((-0.0600000000000000)+(((-2.00000000000000)*(py)))))):(IKReal)1.0e30);
IKReal x7=((2.00000000000000)*(px)*(x6));
if( (((-0.00360000000000000)+(((4.00000000000000)*((py)*(py))))+(((4.00000000000000)*((px)*(px)))))) < (IKReal)-0.00001 )
    continue;
IKReal x8=((x6)*(IKsqrt(((-0.00360000000000000)+(((4.00000000000000)*((py)*(py))))+(((4.00000000000000)*((px)*(px))))))));
j13array[0]=((2.00000000000000)*(atan(((x7)+(((-1.00000000000000)*(x8)))))));
sj13array[0]=IKsin(j13array[0]);
cj13array[0]=IKcos(j13array[0]);
j13array[1]=((2.00000000000000)*(atan(((x7)+(x8)))));
sj13array[1]=IKsin(j13array[1]);
cj13array[1]=IKcos(j13array[1]);
if( j13array[0] > IKPI )
{
    j13array[0]-=IK2PI;
}
else if( j13array[0] < -IKPI )
{    j13array[0]+=IK2PI;
}
j13valid[0] = true;
if( j13array[1] > IKPI )
{
    j13array[1]-=IK2PI;
}
else if( j13array[1] < -IKPI )
{    j13array[1]+=IK2PI;
}
j13valid[1] = true;
for(int ij13 = 0; ij13 < 2; ++ij13)
{
if( !j13valid[ij13] )
{
    continue;
}
_ij13[0] = ij13; _ij13[1] = -1;
for(int iij13 = ij13+1; iij13 < 2; ++iij13)
{
if( j13valid[iij13] && IKabs(cj13array[ij13]-cj13array[iij13]) < IKFAST_SOLUTION_THRESH && IKabs(sj13array[ij13]-sj13array[iij13]) < IKFAST_SOLUTION_THRESH )
{
    j13valid[iij13]=false; _ij13[1] = iij13; break; 
}
}
j13 = j13array[ij13]; cj13 = cj13array[ij13]; sj13 = sj13array[ij13];

{
IKReal dummyeval[1];
IKReal x9=(sj13)*(sj13);
dummyeval[0]=((((4.53333333333333)*(cj13)*(sj13)))+(((-151.111111111111)*(py)*(sj13)))+((cj13)*(cj13))+(((1111.11111111111)*(x9)*((pz)*(pz))))+(((-66.6666666666667)*(cj13)*(py)))+(((5.13777777777778)*(x9)))+(((1111.11111111111)*((py)*(py)))));
if( IKabs(dummyeval[0]) < 0.0000010000000000  )
{
{
IKReal dummyeval[1];
IKReal x10=(cj13)*(cj13);
IKReal x11=(px)*(px);
IKReal x12=((1111.11111111111)*(x11));
dummyeval[0]=((-1.00000000000000)+(((1111.11111111111)*(x10)*((py)*(py))))+(((-1.00000000000000)*(x10)*(x12)))+(x12)+(((-2222.22222222222)*(cj13)*(px)*(py)*(sj13))));
if( IKabs(dummyeval[0]) < 0.0000010000000000  )
{
continue;

} else
{
{
IKReal j14array[4], cj14array[4], sj14array[4];
bool j14valid[4]={false};
_nj14 = 4;
IKReal x13=(cj13)*(cj13);
IKReal x14=(py)*(py);
IKReal x15=(px)*(px);
IKReal x16=((x13)*(x14));
IKReal x17=((2.00000000000000)*(cj13)*(px)*(py)*(sj13));
IKReal x18=((1.00000000000000)*(x13)*(x15));
IKReal x19=((IKabs(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-1.00000000000000)*(x17)))+(x15)+(x16))) != 0)?((IKReal)1/(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-1.00000000000000)*(x17)))+(x15)+(x16)))):(IKReal)1.0e30);
if( (((((-1.00000000000000)*(x17)*(((IKabs(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-2.00000000000000)*(cj13)*(px)*(py)*(sj13)))+(x15)+(x16))) != 0)?((IKReal)1/(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-2.00000000000000)*(cj13)*(px)*(py)*(sj13)))+(x15)+(x16)))):(IKReal)1.0e30))))+(((x16)*(((IKabs(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-1.00000000000000)*(x17)))+(x15)+(((x13)*(x14))))) != 0)?((IKReal)1/(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-1.00000000000000)*(x17)))+(x15)+(((x13)*(x14)))))):(IKReal)1.0e30))))+(((-0.000900000000000000)*(x19)))+(((x15)*(x19)))+(((-1.00000000000000)*(x18)*(((IKabs(((-0.000900000000000000)+(((-1.00000000000000)*(x17)))+(((-1.00000000000000)*(x13)*(x15)))+(x15)+(x16))) != 0)?((IKReal)1/(((-0.000900000000000000)+(((-1.00000000000000)*(x17)))+(((-1.00000000000000)*(x13)*(x15)))+(x15)+(x16)))):(IKReal)1.0e30)))))) < (IKReal)-0.00001 )
    continue;
IKReal x20=IKsqrt(((((-1.00000000000000)*(x17)*(((IKabs(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-2.00000000000000)*(cj13)*(px)*(py)*(sj13)))+(x15)+(x16))) != 0)?((IKReal)1/(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-2.00000000000000)*(cj13)*(px)*(py)*(sj13)))+(x15)+(x16)))):(IKReal)1.0e30))))+(((x16)*(((IKabs(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-1.00000000000000)*(x17)))+(x15)+(((x13)*(x14))))) != 0)?((IKReal)1/(((-0.000900000000000000)+(((-1.00000000000000)*(x18)))+(((-1.00000000000000)*(x17)))+(x15)+(((x13)*(x14)))))):(IKReal)1.0e30))))+(((-0.000900000000000000)*(x19)))+(((x15)*(x19)))+(((-1.00000000000000)*(x18)*(((IKabs(((-0.000900000000000000)+(((-1.00000000000000)*(x17)))+(((-1.00000000000000)*(x13)*(x15)))+(x15)+(x16))) != 0)?((IKReal)1/(((-0.000900000000000000)+(((-1.00000000000000)*(x17)))+(((-1.00000000000000)*(x13)*(x15)))+(x15)+(x16)))):(IKReal)1.0e30))))));
cj14array[0]=x20;
cj14array[2]=((-1.00000000000000)*(x20));
if( cj14array[0] >= -1-IKFAST_SINCOS_THRESH && cj14array[0] <= 1+IKFAST_SINCOS_THRESH )
{
    j14valid[0] = j14valid[1] = true;
    j14array[0] = IKacos(cj14array[0]);
    sj14array[0] = IKsin(j14array[0]);
    cj14array[1] = cj14array[0];
    j14array[1] = -j14array[0];
    sj14array[1] = -sj14array[0];
}
else if( isnan(cj14array[0]) )
{
    // probably any value will work
    j14valid[0] = true;
    cj14array[0] = 1; sj14array[0] = 0; j14array[0] = 0;
}
if( cj14array[2] >= -1-IKFAST_SINCOS_THRESH && cj14array[2] <= 1+IKFAST_SINCOS_THRESH )
{
    j14valid[2] = j14valid[3] = true;
    j14array[2] = IKacos(cj14array[2]);
    sj14array[2] = IKsin(j14array[2]);
    cj14array[3] = cj14array[2];
    j14array[3] = -j14array[2];
    sj14array[3] = -sj14array[2];
}
else if( isnan(cj14array[2]) )
{
    // probably any value will work
    j14valid[2] = true;
    cj14array[2] = 1; sj14array[2] = 0; j14array[2] = 0;
}
for(int ij14 = 0; ij14 < 4; ++ij14)
{
if( !j14valid[ij14] )
{
    continue;
}
_ij14[0] = ij14; _ij14[1] = -1;
for(int iij14 = ij14+1; iij14 < 4; ++iij14)
{
if( j14valid[iij14] && IKabs(cj14array[ij14]-cj14array[iij14]) < IKFAST_SOLUTION_THRESH && IKabs(sj14array[ij14]-sj14array[iij14]) < IKFAST_SOLUTION_THRESH )
{
    j14valid[iij14]=false; _ij14[1] = iij14; break; 
}
}
j14 = j14array[ij14]; cj14 = cj14array[ij14]; sj14 = sj14array[ij14];
{
IKReal evalcond[3];
IKReal x21=IKsin(j14);
IKReal x22=IKcos(j14);
IKReal x23=(cj13)*(cj13);
IKReal x24=(pz)*(pz);
IKReal x25=(py)*(py);
IKReal x26=((cj13)*(sj13));
IKReal x27=(x22)*(x22);
IKReal x28=(x21)*(x21)*(x21);
IKReal x29=((py)*(x21));
IKReal x30=((0.0133280000000000)*(x23));
IKReal x31=((0.0300000000000000)*(x21));
IKReal x32=((sj13)*(x21));
IKReal x33=((pz)*(sj13)*(x22));
IKReal x34=((x24)*(x27));
IKReal x35=((x21)*(x27));
evalcond[0]=((((cj13)*(x29)))+(((-1.00000000000000)*(x31)))+(((-1.00000000000000)*(px)*(x32))));
evalcond[1]=((((-0.0980000000000000)*(sj13)))+(x33)+(x29)+(((-1.00000000000000)*(cj13)*(x31)))+(((-0.0680000000000000)*(x32))));
evalcond[2]=((0.0142280000000000)+(((-1.00000000000000)*(x34)))+(((-0.00408000000000000)*(x26)*(x27)))+(((-2.00000000000000)*(x29)*(x33)))+(((x23)*(x34)))+(((-1.00000000000000)*(x30)*(x35)))+(((0.0133280000000000)*(x35)))+(((-1.00000000000000)*(x28)*(x30)))+(((-1.00000000000000)*(x25)))+(((0.0133280000000000)*(x28)))+(((-1.00000000000000)*(x30)))+(((0.00588000000000000)*(x26)*(x28)))+(((-0.00462400000000000)*(x27)))+(((0.00408000000000000)*(x26)))+(((x25)*(x27)))+(((0.00372400000000000)*(x23)*(x27)))+(((0.00588000000000000)*(x26)*(x35))));
if( IKabs(evalcond[0]) > 0.000001  || IKabs(evalcond[1]) > 0.000001  || IKabs(evalcond[2]) > 0.000001  )
{
continue;
}
}

IKReal soleval[1];
soleval[0]=((-0.0232000000000000)+(((cj14)*(((-0.0680000000000000)+(((cj13)*(px)))+(((py)*(sj13)))))))+(((-1.00000000000000)*(pz)*(sj14))));
if( soleval[0] > 0.0000000000000000  )
{
vsolutions.push_back(IKSolution()); IKSolution& solution = vsolutions.back();
solution.basesol.resize(3);
solution.basesol[0].foffset = j12;
solution.basesol[0].indices[0] = _ij12[0];
solution.basesol[0].indices[1] = _ij12[1];
solution.basesol[0].maxsolutions = _nj12;
solution.basesol[1].foffset = j13;
solution.basesol[1].indices[0] = _ij13[0];
solution.basesol[1].indices[1] = _ij13[1];
solution.basesol[1].maxsolutions = _nj13;
solution.basesol[2].foffset = j14;
solution.basesol[2].indices[0] = _ij14[0];
solution.basesol[2].indices[1] = _ij14[1];
solution.basesol[2].maxsolutions = _nj14;
solution.vfree.resize(0);
}
}
}

}

}

} else
{
{
IKReal j14array[2], cj14array[2], sj14array[2];
bool j14valid[2]={false};
_nj14 = 2;
IKReal x36=((((-0.0680000000000000)*(sj13)))+(py)+(((-0.0300000000000000)*(cj13))));
if( IKabs(((pz)*(sj13))) < IKFAST_ATAN2_MAGTHRESH && IKabs(x36) < IKFAST_ATAN2_MAGTHRESH )
    continue;
IKReal x37=((1.00000000000000)*(IKatan2(((pz)*(sj13)), x36)));
if( ((((((pz)*(pz))*((sj13)*(sj13))))+((x36)*(x36)))) < (IKReal)-0.00001 )
    continue;
if( (((0.0980000000000000)*(sj13)*(((IKabs(IKabs(IKsqrt((((((pz)*(pz))*((sj13)*(sj13))))+((x36)*(x36)))))) != 0)?((IKReal)1/(IKabs(IKsqrt((((((pz)*(pz))*((sj13)*(sj13))))+((x36)*(x36))))))):(IKReal)1.0e30)))) < -1-IKFAST_SINCOS_THRESH || (((0.0980000000000000)*(sj13)*(((IKabs(IKabs(IKsqrt((((((pz)*(pz))*((sj13)*(sj13))))+((x36)*(x36)))))) != 0)?((IKReal)1/(IKabs(IKsqrt((((((pz)*(pz))*((sj13)*(sj13))))+((x36)*(x36))))))):(IKReal)1.0e30)))) > 1+IKFAST_SINCOS_THRESH )
    continue;
IKReal x38=IKasin(((0.0980000000000000)*(sj13)*(((IKabs(IKabs(IKsqrt((((((pz)*(pz))*((sj13)*(sj13))))+((x36)*(x36)))))) != 0)?((IKReal)1/(IKabs(IKsqrt((((((pz)*(pz))*((sj13)*(sj13))))+((x36)*(x36))))))):(IKReal)1.0e30))));
j14array[0]=((((-1.00000000000000)*(x37)))+(x38));
sj14array[0]=IKsin(j14array[0]);
cj14array[0]=IKcos(j14array[0]);
j14array[1]=((3.14159265358979)+(((-1.00000000000000)*(x38)))+(((-1.00000000000000)*(x37))));
sj14array[1]=IKsin(j14array[1]);
cj14array[1]=IKcos(j14array[1]);
if( j14array[0] > IKPI )
{
    j14array[0]-=IK2PI;
}
else if( j14array[0] < -IKPI )
{    j14array[0]+=IK2PI;
}
j14valid[0] = true;
if( j14array[1] > IKPI )
{
    j14array[1]-=IK2PI;
}
else if( j14array[1] < -IKPI )
{    j14array[1]+=IK2PI;
}
j14valid[1] = true;
for(int ij14 = 0; ij14 < 2; ++ij14)
{
if( !j14valid[ij14] )
{
    continue;
}
_ij14[0] = ij14; _ij14[1] = -1;
for(int iij14 = ij14+1; iij14 < 2; ++iij14)
{
if( j14valid[iij14] && IKabs(cj14array[ij14]-cj14array[iij14]) < IKFAST_SOLUTION_THRESH && IKabs(sj14array[ij14]-sj14array[iij14]) < IKFAST_SOLUTION_THRESH )
{
    j14valid[iij14]=false; _ij14[1] = iij14; break; 
}
}
j14 = j14array[ij14]; cj14 = cj14array[ij14]; sj14 = sj14array[ij14];
{
IKReal evalcond[3];
IKReal x39=IKsin(j14);
IKReal x40=(px)*(px);
IKReal x41=(cj13)*(cj13);
IKReal x42=(py)*(py);
IKReal x43=(pz)*(pz);
IKReal x44=IKcos(j14);
IKReal x45=((cj13)*(py));
IKReal x46=((2.00000000000000)*(sj13));
IKReal x47=((0.00408000000000000)*(sj13));
IKReal x48=(x44)*(x44);
IKReal x49=(x39)*(x39)*(x39);
IKReal x50=((1.00000000000000)*(x40));
IKReal x51=((0.0133280000000000)*(x39));
IKReal x52=((sj13)*(x39));
IKReal x53=((0.0133280000000000)*(x41));
IKReal x54=((1.00000000000000)*(x42));
IKReal x55=((cj13)*(x48));
IKReal x56=((x43)*(x48));
IKReal x57=((x41)*(x48));
evalcond[0]=((((-0.0300000000000000)*(x39)))+(((x39)*(x45)))+(((-1.00000000000000)*(px)*(x52))));
evalcond[1]=((0.000900000000000000)+(((-0.000900000000000000)*(x48)))+(((-1.00000000000000)*(x41)*(x54)))+(((x40)*(x48)))+(((x40)*(x41)))+(((x42)*(x57)))+(((px)*(x45)*(x46)))+(((-1.00000000000000)*(px)*(x45)*(x46)*(x48)))+(((-1.00000000000000)*(x50)))+(((-1.00000000000000)*(x50)*(x57))));
evalcond[2]=((0.0142280000000000)+(((-1.00000000000000)*(x51)*(x57)))+(((0.0133280000000000)*(x49)))+(((-1.00000000000000)*(x49)*(x53)))+(((0.00372400000000000)*(x57)))+(((-1.00000000000000)*(x47)*(x55)))+(((x42)*(x48)))+(((cj13)*(x47)))+(((x48)*(x51)))+(((-0.00462400000000000)*(x48)))+(((0.00588000000000000)*(cj13)*(sj13)*(x49)))+(((x41)*(x56)))+(((-1.00000000000000)*(x54)))+(((-1.00000000000000)*(x53)))+(((-1.00000000000000)*(py)*(pz)*(x39)*(x44)*(x46)))+(((-1.00000000000000)*(x56)))+(((0.00588000000000000)*(x52)*(x55))));
if( IKabs(evalcond[0]) > 0.000001  || IKabs(evalcond[1]) > 0.000001  || IKabs(evalcond[2]) > 0.000001  )
{
continue;
}
}

IKReal soleval[1];
soleval[0]=((-0.0232000000000000)+(((cj14)*(((-0.0680000000000000)+(((cj13)*(px)))+(((py)*(sj13)))))))+(((-1.00000000000000)*(pz)*(sj14))));
if( soleval[0] > 0.0000000000000000  )
{
vsolutions.push_back(IKSolution()); IKSolution& solution = vsolutions.back();
solution.basesol.resize(3);
solution.basesol[0].foffset = j12;
solution.basesol[0].indices[0] = _ij12[0];
solution.basesol[0].indices[1] = _ij12[1];
solution.basesol[0].maxsolutions = _nj12;
solution.basesol[1].foffset = j13;
solution.basesol[1].indices[0] = _ij13[0];
solution.basesol[1].indices[1] = _ij13[1];
solution.basesol[1].maxsolutions = _nj13;
solution.basesol[2].foffset = j14;
solution.basesol[2].indices[0] = _ij14[0];
solution.basesol[2].indices[1] = _ij14[1];
solution.basesol[2].maxsolutions = _nj14;
solution.vfree.resize(0);
}
}
}

}

}
}
}

}

}
}
return vsolutions.size()>0;
}

};


/// solves the inverse kinematics equations.
/// \param pfree is an array specifying the free joints of the chain.
IKFAST_API bool ik(const IKReal* eetrans, const IKReal* eerot, const IKReal* pfree, std::vector<IKSolution>& vsolutions) {
IKSolver solver;
return solver.ik(eetrans,eerot,pfree,vsolutions);
}

IKFAST_API const char* getKinematicsHash() { return "2640ae411e0c87b03f56bf289296f9d8"; }

IKFAST_API const char* getIKFastVersion() { return "58"; }

#ifdef IKFAST_NAMESPACE
} // end namespace
#endif

#ifndef IKFAST_NO_MAIN
#include <stdio.h>
#include <stdlib.h>
#ifdef IKFAST_NAMESPACE
using namespace IKFAST_NAMESPACE;
#endif
int main(int argc, char** argv)
{
    if( argc != 12+getNumFreeParameters()+1 ) {
        printf("\nUsage: ./ik r00 r01 r02 t0 r10 r11 r12 t1 r20 r21 r22 t2 free0 ...\n\n"
               "Returns the ik solutions given the transformation of the end effector specified by\n"
               "a 3x3 rotation R (rXX), and a 3x1 translation (tX).\n"
               "There are %d free parameters that have to be specified.\n\n",getNumFreeParameters());
        return 1;
    }

    std::vector<IKSolution> vsolutions;
    std::vector<IKReal> vfree(getNumFreeParameters());
    IKReal eerot[9],eetrans[3];
    eerot[0] = atof(argv[1]); eerot[1] = atof(argv[2]); eerot[2] = atof(argv[3]); eetrans[0] = atof(argv[4]);
    eerot[3] = atof(argv[5]); eerot[4] = atof(argv[6]); eerot[5] = atof(argv[7]); eetrans[1] = atof(argv[8]);
    eerot[6] = atof(argv[9]); eerot[7] = atof(argv[10]); eerot[8] = atof(argv[11]); eetrans[2] = atof(argv[12]);
    for(std::size_t i = 0; i < vfree.size(); ++i)
        vfree[i] = atof(argv[13+i]);
    bool bSuccess = ik(eetrans, eerot, vfree.size() > 0 ? &vfree[0] : NULL, vsolutions);

    if( !bSuccess ) {
        fprintf(stderr,"Failed to get ik solution\n");
        return -1;
    }

    printf("Found %d ik solutions:\n", (int)vsolutions.size());
    std::vector<IKReal> sol(getNumJoints());
    for(std::size_t i = 0; i < vsolutions.size(); ++i) {
        printf("sol%d (free=%d): ", (int)i, (int)vsolutions[i].GetFree().size());
        std::vector<IKReal> vsolfree(vsolutions[i].GetFree().size());
        vsolutions[i].GetSolution(&sol[0],vsolfree.size()>0?&vsolfree[0]:NULL);
        for( std::size_t j = 0; j < sol.size(); ++j)
            printf("%.15f, ", sol[j]);
        printf("\n");
    }
    return 0;
}

#endif

#include "ikbase.h"
namespace IKFAST_NAMESPACE {
#ifdef RAVE_REGISTER_BOOST
#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()
BOOST_TYPEOF_REGISTER_TYPE(IKSolution)
#endif
IkSolverBasePtr CreateIkSolver(EnvironmentBasePtr penv, const std::vector<dReal>& vfreeinc) {
    std::vector<int> vfree(getNumFreeParameters());
    for(size_t i = 0; i < vfree.size(); ++i) {
        vfree[i] = getFreeParameters()[i];
    }
    return IkSolverBasePtr(new IkFastSolver<IKReal,IKSolution>(ik,vfree,vfreeinc,getNumJoints(),static_cast<IkParameterizationType>(getIKType()), boost::shared_ptr<void>(), getKinematicsHash(), penv));
}
} // end namespace
