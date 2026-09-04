//
// Utility functions for the mesh processing.
//
#pragma once

#include <lxsdk/lx_log.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lxu_math.hpp>
#include <lxsdk/lxvmath.h>
#include <lxsdk/lxu_matrix.hpp>
#include <lxsdk/lxu_quaternion.hpp>
#include <lxsdk/lx_vp.hpp>

//
// Basic vector math functions.
//
namespace MathUtil {
    static bool Compare(double a, double b)
    {
        double tol = lx::Tolerance(a) * 10.0;
        return std::abs(a - b) < tol;
    }

    static bool VectorCompare (CLxVector& A, CLxVector B)
    {
        if ((MathUtil::Compare(A[0], B[0])) &&
            (MathUtil::Compare(A[1], B[1])) &&
            (MathUtil::Compare(A[2], B[2])))
            return true;
        else
            return false;
    }

    static CLxVector NormalTriangle(LXtFVector v0, LXtFVector v1, LXtFVector vn)
    {
        LXtVector d0, d1, dn;

        LXx_VSUB3(d0, v1, v0);
        LXx_VSUB3(d1, vn, v0);
        LXx_VCROSS(dn, d0, d1);

        CLxVector vec(dn);
        vec.normalize();
        return vec;
    }

    static CLxVector CornerNormal(CLxUser_Mesh& mesh, LXtPolygonID pol, LXtPointID vrt0, LXtPointID vrt1)
    {
        LXtFVector      pos, posN, posP;
        CLxUser_Polygon poly;
        poly.fromMesh(mesh);
        poly.Select(pol);

        CLxUser_Point point;
        point.fromMesh(mesh);

        LXtPointID next, prev;
        unsigned   nvert, i;
        poly.VertexCount(&nvert);

        point.Select(vrt0);
        point.Pos(pos);
        poly.PointIndex(vrt0, &i);
        poly.VertexByIndex((i + 1) % nvert, &next);
        poly.VertexByIndex((i + nvert - 1) % nvert, &prev);
        point.Select(next);
        point.Pos(posN);
        point.Select(prev);
        point.Pos(posP);
        CLxVector norm0 = NormalTriangle(posP, pos, posN);

        point.Select(vrt1);
        point.Pos(pos);
        poly.PointIndex(vrt1, &i);
        poly.VertexByIndex((i + 1) % nvert, &next);
        poly.VertexByIndex((i + nvert - 1) % nvert, &prev);
        point.Select(next);
        point.Pos(posN);
        point.Select(prev);
        point.Pos(posP);
        CLxVector norm1 = NormalTriangle(posP, pos, posN);

        norm0 += norm1;
        norm0.normalize();
        return norm0;
    }

    static double VectorAngle(const LXtVector v0, const LXtVector v1, int normalize = 0)
    {
        double dot = LXx_VDOT (v0, v1);
        if (normalize) {
            dot = dot / (LXx_VLEN (v0) * LXx_VLEN (v1));
        }
        dot = LXxCLAMP(dot, -1.0, 1.0);
        return std::acos (dot);
    }
};