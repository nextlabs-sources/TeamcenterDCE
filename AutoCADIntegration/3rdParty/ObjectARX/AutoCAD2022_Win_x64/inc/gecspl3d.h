//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2021 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// This file contains the class AcGeCubicSplineCurve3d - A mathematic
// entity used to represent a 3d interpolation cubic-spline curve.

#ifndef AC_GECSPL3D_H
#define AC_GECSPL3D_H

class AcGePointOnCurve3d;
class AcGeCurve3dIntersection;
class AcGeInterval;
class AcGePlane;

#include "gesent3d.h"
#include "gept3dar.h"
#include "gevc3dar.h"
#include "gevec3d.h"
#include "gekvec.h"
#pragma pack (push, 8)

class

AcGeCubicSplineCurve3d : public AcGeSplineEnt3d
{
public:

    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d();
    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d(const AcGeCubicSplineCurve3d& spline);

    // Construct a periodic cubic spline curve
    // Contract: the first fit point must be equal to the last fit point
    //
    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d(const AcGePoint3dArray& fitPnts,
			   const AcGeTol& tol = AcGeContext::gTol);

    // Construct a cubic spline curve with clamped end condition
    //
    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d(const AcGePoint3dArray& fitPnts,
			   const AcGeVector3d& startDeriv,
			   const AcGeVector3d& endDeriv,
                           const AcGeTol& tol = AcGeContext::gTol);

    // Construct a cubic spline approximating the curve
    //
    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d(const AcGeCurve3d& curve,
                           double epsilon = AcGeContext::gTol.equalPoint());

    // Construct a cubic spline curve with given fit points and 1st derivatives
    //
    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d(const AcGeKnotVector& knots,
	                   const AcGePoint3dArray& fitPnts,
			   const AcGeVector3dArray& firstDerivs,
	                   Adesk::Boolean isPeriodic = Adesk::kFalse);

    // Definition of spline
    //
    GE_DLLEXPIMPORT int                     numFitPoints   ()        const;
    GE_DLLEXPIMPORT AcGePoint3d             fitPointAt     (int idx) const;
    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d& setFitPointAt  (int idx, const AcGePoint3d& point);
    GE_DLLEXPIMPORT AcGeVector3d            firstDerivAt   (int idx) const;
    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d& setFirstDerivAt(int idx, const AcGeVector3d& deriv);

    // Assignment operator.
    //
    GE_DLLEXPIMPORT AcGeCubicSplineCurve3d&  operator = (const AcGeCubicSplineCurve3d& spline);
};

#pragma pack (pop)
#endif
