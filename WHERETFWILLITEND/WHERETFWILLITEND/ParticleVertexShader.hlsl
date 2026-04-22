struct VS_OUT
{
    uint id : PARTICLE_ID;
};


VS_OUT main(uint vertexID : SV_VertexID)
{
    VS_OUT o;
    o.id = vertexID;
    return o;
}