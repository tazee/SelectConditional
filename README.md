# Conditional Selection tools for Modo plug-in

<b>Conditional Selection Tools</b> are a collection of tools for selecting mesh elements based on specified conditions. The available tools are as follows:

1. <b>Select Edges by Length</b>:<br>
Selects edges based on a specified length (edges longer/shorter than the specified length, or edges with lengths within the range defined by Min and Max). If an edge is selected before launching the tool, its length is used as the initial value for the <b>Length</b> parameter.<br>
2. <b>Select Edges by Normal</b>:<br>
Selects edges based on the average vector of the normals of the polygons sharing the edge. The reference edge is the last edge selected before launching the tool. For edges shared by only a single polygon (boundary edges), the determination is made according to the method specified in <b>Open Edges</b>. If <b>Polygon Normal</b> is selected, the normal vectors of the polygons sharing the edge serve as the reference vectors. For <b>Outward Vector</b>, the cross product of the polygon normal vector and the vector connecting the edge's start and end points is used as the reference vector. Edges with a vector angle relative to the reference vector that is less than or equal to the angle threshold are selected.<br>
3. <b>Select Edges by Edge Vector</b>:<br>
Selects edges based on the edge vector calculated from the edge's start and end points. The edge selected before launching the tool serves as the reference vector. Edges with a vector angle relative to the reference vector that is less than or equal to the angle threshold are selected.<br>
4. <b>Select Polygons by Area</b>:<br>
Selects polygons based on a specified area (polygons with an area larger/smaller than the specified area, or polygons with an area within the range defined by Min and Max). If a polygon is selected before launching the tool, its area is used as the initial value for the <b>Area</b> setting.<br>
5. <b>Random Selection</b>:<br>
Randomly selects mesh elements (vertices, edges, or polygons). The random number generation is determined by the <b>Seed</b> value; changing this value alters the random pattern. Each element is assigned a random weight value ranging from 0.0 to 1.0. Elements with a weight value lower than the specified <b>percentage</b> are selected.<br>

This kit contains a direct modeling tool for Modo macOS and Windows.

<div align="left">
<img src="images/random.gif" style='max-height: 500px; object-fit: contain'/>
</div>

## Installing

- Download lpk from releases. Drag and drop it into your Modo viewport. If you're upgrading, delete previous version.

## How to use Conditional Selection

- Conditional Selection can be launched from **Conditional Selection** button on **Select** tab of **Model** ToolBar on left. The popup panel contains the buttons to launch each tools.

<div align="left">
<img src="images/UI.png" style='max-height: 620px; object-fit: contain'/>
</div>



