struct VS_Output
{
	float3 Tex : TEXCOORD0;
    float4 Pos : SV_POSITION;
};
 
VS_Output main(uint id : SV_VertexID, uint instId : SV_InstanceID)
{
    VS_Output Output;
    Output.Tex = float3((id << 1) & 2, id & 2, instId);
    Output.Pos = float4(Output.Tex.xy * float2(2,-2) + float2(-1,1), 0, 1);
    return Output;
}