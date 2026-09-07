//
// Utility functions for the mesh processing.
//
#pragma once

#include <lxsdk/lx_log.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lx_vp.hpp>
#include <lxsdk/lxu_math.hpp>
#include <lxsdk/lxu_matrix.hpp>
#include <lxsdk/lxu_quaternion.hpp>
#include <lxsdk/lxvmath.h>

#include <lxsdk/lxu_attributes.hpp>
#include <lxsdk/lxu_math.hpp>
#include <lxsdk/lxu_select.hpp>
#include <lxsdk/lxu_vector.hpp>

#include <lxsdk/lx_channelui.hpp>
#include <lxsdk/lx_draw.hpp>
#include <lxsdk/lx_handles.hpp>
#include <lxsdk/lx_layer.hpp>
#include <lxsdk/lx_log.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_plugin.hpp>
#include <lxsdk/lx_pmodel.hpp>
#include <lxsdk/lx_seltypes.hpp>
#include <lxsdk/lx_tool.hpp>
#include <lxsdk/lx_toolui.hpp>
#include <lxsdk/lx_vector.hpp>
#include <lxsdk/lx_vmodel.hpp>
#include <lxsdk/lx_vp.hpp>

#include <lxsdk/lx_select.hpp>
#include <lxsdk/lx_seltypes.hpp>
#include <lxsdk/lx_value.hpp>

//
// Basic vector math functions.
//
namespace MathUtil
{
    static bool Compare(double a, double b)
    {
        double tol = lx::Tolerance(a) * 10.0;
        return std::abs(a - b) < tol;
    }

    static bool VectorCompare(CLxVector& A, CLxVector B)
    {
        if ((MathUtil::Compare(A[0], B[0])) && (MathUtil::Compare(A[1], B[1])) && (MathUtil::Compare(A[2], B[2])))
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
        double dot = LXx_VDOT(v0, v1);
        if (normalize)
        {
            dot = dot / (LXx_VLEN(v0) * LXx_VLEN(v1));
        }
        dot = LXxCLAMP(dot, -1.0, 1.0);
        return std::acos(dot);
    }
};  // namespace MathUtil
namespace ModoUtil
{
    static void GetWorkPlane(LXtMatrix4 m4)
    {
        lx::Matrix4Ident(m4);
        CLxUser_SelectionService s_sel;
        LXtID4 selID_scene = s_sel.LookupType(LXsSELTYP_SCENE);
        CLxUser_ScenePacketTranslation scenePkt;
        scenePkt.autoInit();

        void* pkt = s_sel.Recent(selID_scene);
        if (pkt != nullptr)
        {
            CLxUser_ChannelRead  chanRead;
            LXtVector            wpPos;
            LXtMatrix            wpRot;
            CLxUser_Scene      scene;
        
            scenePkt.GetScene(pkt, scene);
            scene.GetChannels(chanRead, 0.0);
            scene.WorkPlaneRotation(chanRead, wpRot);
            scene.WorkPlanePosition(chanRead, wpPos);
            /*
            printf("wpPos: %f, %f, %f\n", wpPos[0], wpPos[1], wpPos[2]);
            printf("wpRot: %f, %f, %f\n", wpRot[0][0], wpRot[0][1], wpRot[0][2]);
            printf("       %f, %f, %f\n", wpRot[1][0], wpRot[1][1], wpRot[1][2]);
            printf("       %f, %f, %f\n", wpRot[2][0], wpRot[2][1], wpRot[2][2]);
            */
            lx::Matrix4SetSubMatrix(m4, wpRot, 0);
            lx::MatrixTranspose(wpRot);
            lx::MatrixMultiply(m4[3], wpRot, wpPos);
            LXx_VNEG(m4[3]);
            /*
            printf("m4 : %f, %f, %f, %f\n", m4[0][0], m4[0][1], m4[0][2], m4[0][3]);
            printf("     %f, %f, %f, %f\n", m4[1][0], m4[1][1], m4[1][2], m4[1][3]);
            printf("     %f, %f, %f, %f\n", m4[2][0], m4[2][1], m4[2][2], m4[2][3]);
            printf("     %f, %f, %f, %f\n", m4[3][0], m4[3][1], m4[3][2], m4[3][3]);
            LXtMatrix m3;
            lx::Matrix4GetSubMatrix(m4, m3, 0);
            printf("m3: %f, %f, %f\n", m3[0][0], m3[0][1], m3[0][2]);
            printf("    %f, %f, %f\n", m3[1][0], m3[1][1], m3[1][2]);
            printf("    %f, %f, %f\n", m3[2][0], m3[2][1], m3[2][2]);
            */
        }
    }
};
