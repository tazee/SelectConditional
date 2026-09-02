//
// SelectPolysByArea - A plugin for selecting polygons by area
//

#include "polyByArea.hpp"
#include "util.hpp"

namespace PolyByArea
{

    static const char* SRVNAME_TOOL = "select.polysByArea";

#define ATTRs_AREA     "area"
#define ATTRs_OPERATOR "operator"
#define ATTRs_DESELECT "deselect"

#define ATTRa_AREA     0
#define ATTRa_OPERATOR 1
#define ATTRa_DESELECT 2

    /*
     * On create we add our one tool attribute. We also allocate a vector type
     * and select mode mask.
     */
    CSelectPolysByArea::CSelectPolysByArea()
    {
        CLxUser_PacketService sPkt;
        CLxUser_MeshService   sMesh;

        static const LXtTextValueHint comparison_operators[] = {
            { OPERATOR_LESSTHAN, "less_than" }, { OPERATOR_EQUAL, "equal" }, { OPERATOR_GREATERTHAN, "greater_than" }, { 0, "=comparison_operators" }, 0
        };

        dyna_Add(ATTRs_AREA, LXsTYPE_DISTANCE);
        dyna_Add(ATTRs_OPERATOR, LXsTYPE_INTEGER);
        dyna_SetHint(ATTRa_OPERATOR, comparison_operators);
        dyna_Add(ATTRs_DESELECT, LXsTYPE_BOOLEAN);

        tool_Reset();

        sPkt.NewVectorType(LXsCATEGORY_TOOL, v_type);
        sPkt.AddPacket(v_type, LXsP_TOOL_VIEW_EVENT, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_SCREEN_EVENT, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_FALLOFF, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_SUBJECT2, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_INPUT_EVENT, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_EVENTTRANS, LXfVT_GET);
        sPkt.AddPacket(v_type, LXsP_TOOL_ACTCENTER, LXfVT_GET);

        offset_view    = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_VIEW_EVENT);
        offset_screen  = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_SCREEN_EVENT);
        offset_falloff = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_FALLOFF);
        offset_subject = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_SUBJECT2);
        offset_input   = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_INPUT_EVENT);
        offset_event   = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_EVENTTRANS);
        offset_center  = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_ACTCENTER);
        offset_xfrm    = sPkt.GetOffset(LXsCATEGORY_TOOL, LXsP_TOOL_XFRM);
        mode_select    = sMesh.SetMode("select");
    }

    /*
     * Reset sets the attributes back to defaults.
     */
    void CSelectPolysByArea::tool_Reset()
    {
        dyna_Value(ATTRa_AREA).SetFlt(0.0);
        dyna_Value(ATTRa_OPERATOR).SetInt(OPERATOR_EQUAL);
        dyna_Value(ATTRa_DESELECT).SetInt(0);
    }

    /*
     * Boilerplate methods that identify this as an action (state altering) tool.
     */
    LXtObjectID CSelectPolysByArea::tool_VectorType()
    {
        return v_type.m_loc;  // peek method; does not add-ref
    }

    const char* CSelectPolysByArea::tool_Order()
    {
        return LXs_ORD_ACTR;
    }

    LXtID4 CSelectPolysByArea::tool_Task()
    {
        return LXi_TASK_ACTR;
    }

    /*
     * We employ the simplest possible tool model -- default hauling. We indicate
     * that we want to haul one attribute, we name the attribute, and we implement
     * Initialize() which is what to do when the tool activates or re-activates.
     * In this case set the axis to the current value.
     */
    unsigned CSelectPolysByArea::tmod_Flags()
    {
        return LXfTMOD_I0_ATTRHAUL;
    }

    LxResult CSelectPolysByArea::tmod_Enable(ILxUnknownID obj)
    {
        CLxUser_Message msg(obj);
        unsigned int    primary_index = 0;

        if (TestVertex(primary_index) == false)
        {
            msg.SetCode(LXe_CMD_DISABLED);
            msg.SetMessage(SRVNAME_TOOL, "NoVertex", 0);
            return LXe_DISABLED;
        }
        return LXe_OK;
    }

    const char* CSelectPolysByArea::tmod_Haul(unsigned index)
    {
        if (index == 0)
            return ATTRs_AREA;
        else
            return nullptr;
    }

    void CSelectPolysByArea::tmod_Initialize(ILxUnknownID vts, ILxUnknownID adjust, unsigned int flags)
    {
        CLxUser_AdjustTool at(adjust);
        int                count = s_sel.Count(LXiSEL_POLYGON);
        if (count == 0)
        {
            at.SetFlt(ATTRa_AREA, 0.0);
            return;
        }
        void*                            pkt = s_sel.ByIndex(LXiSEL_POLYGON, static_cast<unsigned>(count - 1));
        CLxUser_PolygonPacketTranslation poly_pkt_trans;
        poly_pkt_trans.autoInit();
        LXtPolygonID pol;
        poly_pkt_trans.Polygon(pkt, &pol);
        CLxUser_Mesh mesh;
        poly_pkt_trans.GetMesh(pkt, mesh);
        CLxUser_Mesh    inst = GetInstance(mesh);
        CLxUser_Polygon poly;
        poly.fromMesh(inst);
        poly.Select(pol);
        double area;
        poly.Area(&area);
        at.SetFlt(ATTRa_AREA, area);
    }

    LxResult CSelectPolysByArea::atrui_DisableMsg(unsigned int index, ILxUnknownID msg)
    {
        return LXe_OK;
    }

    LxResult CSelectPolysByArea::atrui_UIHints(unsigned int index, ILxUnknownID hints)
    {
        CLxLoc_UIHints uiHints(hints);

        if (index == ATTRa_AREA)
        {
            uiHints.MinFloat(0.0);
        }
        return LXe_OK;
    }

    bool CSelectPolysByArea::TestVertex(unsigned int& primary_index)
    {
        /*
         * Start the scan in read-only mode.
         */
        CLxUser_LayerScan scan;
        CLxUser_Mesh      mesh;
        unsigned          i, n, count;
        bool              ok = false;

        primary_index = 0;

        s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE, scan);

        if (scan)
        {
            n = scan.NumLayers();
            for (i = 0; i < n; i++)
            {
                scan.BaseMeshByIndex(i, mesh);
                mesh.PolygonCount(&count);
                if (count > 0)
                {
                    ok            = true;
                    primary_index = i;
                    break;
                }
            }
            scan.Apply();
        }

        /*
         * Return false if there is no polygons in any active layers.
         */
        return ok;
    }

    CLxUser_Mesh CSelectPolysByArea::GetInstance(CLxUser_Mesh& base)
    {
        CLxUser_LayerScan scan;
        s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE, scan);
        auto n = scan.NumLayers();
        for (auto i = 0u; i < n; i++)
        {
            CLxUser_Mesh mesh;
            scan.MeshBase(i, mesh);
            if (mesh_svc.MeshToMeshID(mesh) == mesh_svc.MeshToMeshID(base))
            {
                scan.MeshInstance(i, mesh);
                return mesh;
            }
        }
        return base;
    }

    class PolyVisitor : public CLxImpl_AbstractVisitor
    {
    public:
        bool Test(double area)
        {
            if (m_operator == CSelectPolysByArea::OPERATOR_EQUAL)
            {
                if (MathUtil::Compare(area, m_area))
                    return true;
            }
            else if (m_operator == CSelectPolysByArea::OPERATOR_LESSTHAN)
            {
                if (area < m_area)
                    return true;
            }
            else if (m_operator == CSelectPolysByArea::OPERATOR_GREATERTHAN)
            {
                if (area > m_area)
                    return true;
            }
            return false;
        }

        LxResult Evaluate()
        {
            double area;
            m_poly.Area(&area);

            if (Test(area) == true)
            {
                m_polys.push_back(m_poly.ID());
                return LXe_OK;
            }

            return LXe_OK;
        }

        CLxUser_Mesh              m_mesh;
        CLxUser_Polygon           m_poly;
        CLxUser_Point             m_vert;
        LXtMarkMode               m_mark_pick;
        double                    m_area;
        int                       m_operator;
        std::vector<LXtPolygonID> m_polys;
    };

    /*
     * Tool evaluation uses layer scan interface to walk through all the active
     * meshes and visit all the selected polygons.
     */
    void CSelectPolysByArea::tool_Evaluate(ILxUnknownID vts)
    {
        std::cout << "CSelectPolysByArea::tool_Evaluate: " << std::endl;

        CLxUser_VectorStack    vec(vts);
        CLxUser_Subject2Packet subject;
        if (vec.ReadObject(offset_subject, subject) == false)
            return;

        LXpToolViewEvent* viewEvent = (LXpToolViewEvent*) vec.Read(offset_view);
        if (!viewEvent || viewEvent->type != LXi_VIEWTYPE_3D)
            return;

        PolyVisitor vis;
        dyna_Value(ATTRa_AREA).GetFlt(&vis.m_area);
        dyna_Value(ATTRa_OPERATOR).GetInt(&vis.m_operator);

        if (vis.m_area < 0.0)
        {
            vis.m_area = 0.0;
            dyna_Value(ATTRa_AREA).SetFlt(vis.m_area);
        }

        CLxUser_LayerScan scan;
        s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE | LXf_LAYERSCAN_MARKPOLYS | LXf_LAYERSCAN_MARKVERTS, scan);
        auto n = scan.NumLayers();

        CLxUser_PolygonPacketTranslation poly_pkt_trans;
        poly_pkt_trans.autoInit();

        int deselect;
        dyna_Value(ATTRa_DESELECT).GetInt(&deselect);

        s_sel.StartBatch();

        for (auto i = 0u; i < n; i++)
        {
            CLxUser_Mesh mesh;
            scan.MeshInstance(i, mesh);
            vis.m_mesh = mesh;
            vis.m_poly.fromMesh(mesh);
            vis.m_vert.fromMesh(mesh);
            vis.m_mark_pick = mesh_svc.SetMode(LXsMARK_SELECT);

            vis.m_poly.Enum(&vis, LXiMARK_ANY);

            for (auto j = 0u; j < vis.m_polys.size(); j++)
            {
                void* pkt = poly_pkt_trans.Packet(vis.m_polys[j], mesh);
                if (pkt)
                {
                    if (deselect)
                        s_sel.Deselect(LXiSEL_POLYGON, pkt);
                    else
                        s_sel.Select(LXiSEL_POLYGON, pkt);
                }
            }
        }
        s_sel.EndBatch();

        if (subject.Type() != LXiSEL_POLYGON)
        {
            CLxUser_CommandService cmdSvc;
            cmdSvc.ExecuteArgString(-1, LXiCTAG_NULL, "select.type polygon");
        }
    }

    /*
     * Export tool server.
     */
    void initialize()
    {
        CLxGenericPolymorph* srv;

        srv = new CLxPolymorph<CSelectPolysByArea>;
        srv->AddInterface(new CLxIfc_Tool<CSelectPolysByArea>);
        srv->AddInterface(new CLxIfc_ToolModel<CSelectPolysByArea>);
        srv->AddInterface(new CLxIfc_Attributes<CSelectPolysByArea>);
        srv->AddInterface(new CLxIfc_AttributesUI<CSelectPolysByArea>);
        srv->AddInterface(new CLxIfc_ChannelUI<CSelectPolysByArea>);
        lx::AddServer(SRVNAME_TOOL, srv);
    }
};  // namespace PolyByArea
