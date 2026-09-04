//
// Random - A plugin for selecting mesh elements at random
//

#include "random.hpp"

#include <numeric>
#include <random>

namespace Random
{

    static const char* SRVNAME_TOOL = "select.random";

#define ATTRs_SEED       "seed"
#define ATTRs_PERCENTAGE "percentage"
#define ATTRs_DESELECT "deselect"

#define ATTRa_SEED       0
#define ATTRa_PERCENTAGE 1
#define ATTRa_DESELECT   2

    /*
     * On create we add our one tool attribute. We also allocate a vector type
     * and select mode mask.
     */
    CRandom::CRandom()
    {
        CLxUser_PacketService sPkt;
        CLxUser_MeshService   sMesh;

        dyna_Add(ATTRs_SEED, LXsTYPE_INTEGER);
        dyna_Add(ATTRs_PERCENTAGE, LXsTYPE_PERCENT);
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
    void CRandom::tool_Reset()
    {
        dyna_Value(ATTRa_SEED).SetInt(1074);
        dyna_Value(ATTRa_PERCENTAGE).SetFlt(0.5);
        dyna_Value(ATTRa_DESELECT).SetInt(0);
    }

    /*
     * Boilerplate methods that identify this as an action (state altering) tool.
     */
    LXtObjectID CRandom::tool_VectorType()
    {
        return v_type.m_loc;  // peek method; does not add-ref
    }

    const char* CRandom::tool_Order()
    {
        return LXs_ORD_ACTR;
    }

    LXtID4 CRandom::tool_Task()
    {
        return LXi_TASK_ACTR;
    }

    /*
     * We employ the simplest possible tool model -- default hauling. We indicate
     * that we want to haul one attribute, we name the attribute, and we implement
     * Initialize() which is what to do when the tool activates or re-activates.
     * In this case set the axis to the current value.
     */
    unsigned CRandom::tmod_Flags()
    {
        return LXfTMOD_I0_ATTRHAUL;
    }

    LxResult CRandom::tmod_Enable(ILxUnknownID obj)
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

    const char* CRandom::tmod_Haul(unsigned index)
    {
        if (index == 0)
            return ATTRs_PERCENTAGE;
        else
            return nullptr;
    }

    void CRandom::tmod_Initialize(ILxUnknownID vts, ILxUnknownID adjust, unsigned int flags)
    {
    }

    LxResult CRandom::atrui_DisableMsg(unsigned int index, ILxUnknownID msg)
    {
        return LXe_OK;
    }

    LxResult CRandom::atrui_UIHints(unsigned int index, ILxUnknownID hints)
    {
        CLxLoc_UIHints uiHints(hints);

        if (index == ATTRa_PERCENTAGE)
        {
            uiHints.MinFloat(0.0);
            uiHints.MaxFloat(1.0);
        }
        if (index == ATTRa_SEED)
        {
            uiHints.MinInt(0);
        }
        return LXe_OK;
    }

    bool CRandom::TestVertex(unsigned int& primary_index)
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

    CLxUser_Mesh CRandom::GetInstance(CLxUser_Mesh& base)
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

    class RandomVisitor : public CLxImpl_AbstractVisitor
    {
    public:
        LxResult Evaluate()
        {
            if (m_type == LXiSEL_POLYGON)
                m_polys.push_back(m_poly.ID());
            else if (m_type == LXiSEL_EDGE)
                m_edges.push_back(m_edge.ID());
            else if (m_type == LXiSEL_VERTEX)
                m_verts.push_back(m_vert.ID());
            return LXe_OK;
        }

        CLxUser_Mesh              m_mesh;
        CLxUser_Polygon           m_poly;
        CLxUser_Edge              m_edge;
        CLxUser_Point             m_vert;
        LXtMarkMode               m_mark_pick;
        std::vector<LXtPolygonID> m_polys;
        std::vector<LXtEdgeID>    m_edges;
        std::vector<LXtPointID>   m_verts;
        LXtID4                    m_type;
    };

    /*
     * Tool evaluation uses layer scan interface to walk through all the active
     * meshes and visit all the selected polygons.
     */
    void CRandom::tool_Evaluate(ILxUnknownID vts)
    {
        std::cout << "CRandom::tool_Evaluate: " << std::endl;

        CLxUser_VectorStack    vec(vts);
        CLxUser_Subject2Packet subject;
        if (vec.ReadObject(offset_subject, subject) == false)
            return;

        LXpToolViewEvent* viewEvent = (LXpToolViewEvent*) vec.Read(offset_view);
        if (!viewEvent || viewEvent->type != LXi_VIEWTYPE_3D)
            return;

        int    seed, deselect;
        double percentage;
        dyna_Value(ATTRa_SEED).GetInt(&seed);
        dyna_Value(ATTRa_PERCENTAGE).GetFlt(&percentage);
        dyna_Value(ATTRa_DESELECT).GetInt(&deselect);
        if (percentage < 0.0)
        {
            percentage = 0.0;
            dyna_Value(ATTRa_PERCENTAGE).SetFlt(0.0);
        }
        else if (percentage > 1.0)
        {
            percentage = 1.0;
            dyna_Value(ATTRa_PERCENTAGE).SetFlt(1.0);
        }

        CLxUser_LayerScan scan;
        s_layer.BeginScan(LXf_LAYERSCAN_ACTIVE | LXf_LAYERSCAN_MARKALL, scan);
        auto n = scan.NumLayers();

        CLxUser_PolygonPacketTranslation poly_pkt_trans;
        poly_pkt_trans.autoInit();

        CLxUser_EdgePacketTranslation edge_pkt_trans;
        edge_pkt_trans.autoInit();

        CLxUser_VertexPacketTranslation vert_pkt_trans;
        vert_pkt_trans.autoInit();

        s_sel.StartBatch();

        for (auto i = 0u; i < n; i++)
        {
            CLxUser_Mesh mesh;
            scan.MeshInstance(i, mesh);
            RandomVisitor vis;
            vis.m_type      = subject.Type();
            vis.m_mesh      = mesh;
            vis.m_mark_pick = mesh_svc.SetMode(LXsMARK_SELECT);

            if (vis.m_type == LXiSEL_POLYGON)
            {
                vis.m_poly.fromMesh(mesh);
                vis.m_poly.Enum(&vis, LXiMARK_ANY);

                if (vis.m_polys.empty())
                    continue;

                std::vector<int> data(vis.m_polys.size());
                std::iota(data.begin(), data.end(), 0);

                std::vector<int> combined_seed;
                combined_seed.push_back(seed);
                combined_seed.insert(combined_seed.end(), data.begin(), data.end());
                std::seed_seq                          seq(combined_seed.begin(), combined_seed.end());
                std::mt19937                           engine(seq);
                std::uniform_real_distribution<double> dist(0.0, 1.0);

                for (auto j = 0u; j < vis.m_polys.size(); j++)
                {
                    if (dist(engine) <= percentage)
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
            }
            else if (vis.m_type == LXiSEL_EDGE)
            {
                vis.m_edge.fromMesh(mesh);
                vis.m_edge.Enum(&vis, LXiMARK_ANY);

                if (vis.m_edges.empty())
                    continue;

                std::vector<int> data(vis.m_edges.size());
                std::iota(data.begin(), data.end(), 0);

                std::vector<int> combined_seed;
                combined_seed.push_back(seed);
                combined_seed.insert(combined_seed.end(), data.begin(), data.end());
                std::seed_seq                          seq(combined_seed.begin(), combined_seed.end());
                std::mt19937                           engine(seq);
                std::uniform_real_distribution<double> dist(0.0, 1.0);

                for (auto j = 0u; j < vis.m_edges.size(); j++)
                {
                    if (dist(engine) <= percentage)
                    {
                        vis.m_edge.Select(vis.m_edges[j]);
                        LXtPointID  vrt0, vrt1;
                        vis.m_edge.Endpoints(&vrt0, &vrt1); 
                        void* pkt = edge_pkt_trans.Packet(vrt0, vrt1, nullptr, mesh);
                        if (pkt)
                        {
                            if (deselect)
                                s_sel.Deselect(LXiSEL_EDGE, pkt);
                            else
                                s_sel.Select(LXiSEL_EDGE, pkt);
                        }
                    }
                }
            }
            else if (vis.m_type == LXiSEL_VERTEX)
            {
                vis.m_vert.fromMesh(mesh);
                vis.m_vert.Enum(&vis, LXiMARK_ANY);

                if (vis.m_verts.empty())
                    continue;

                std::vector<int> data(vis.m_verts.size());
                std::iota(data.begin(), data.end(), 0);

                std::vector<int> combined_seed;
                combined_seed.push_back(seed);
                combined_seed.insert(combined_seed.end(), data.begin(), data.end());
                std::seed_seq                          seq(combined_seed.begin(), combined_seed.end());
                std::mt19937                           engine(seq);
                std::uniform_real_distribution<double> dist(0.0, 1.0);

                for (auto j = 0u; j < vis.m_verts.size(); j++)
                {
                    if (dist(engine) <= percentage)
                    {
                        LXtPointID vrt = vis.m_verts[j];
                        void* pkt = vert_pkt_trans.Packet(vrt, nullptr, mesh);
                        if (pkt)
                        {
                            if (deselect)
                                s_sel.Deselect(LXiSEL_VERTEX, pkt);
                            else
                                s_sel.Select(LXiSEL_VERTEX, pkt);
                        }
                    }
                }
            }
        }

        s_sel.EndBatch();
    }

    /*
     * Export tool server.
     */
    void initialize()
    {
        CLxGenericPolymorph* srv;

        srv = new CLxPolymorph<CRandom>;
        srv->AddInterface(new CLxIfc_Tool<CRandom>);
        srv->AddInterface(new CLxIfc_ToolModel<CRandom>);
        srv->AddInterface(new CLxIfc_Attributes<CRandom>);
        srv->AddInterface(new CLxIfc_AttributesUI<CRandom>);
        srv->AddInterface(new CLxIfc_ChannelUI<CRandom>);
        lx::AddServer(SRVNAME_TOOL, srv);
    }
};  // namespace Random
