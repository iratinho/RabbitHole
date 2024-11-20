struct FragmentInput {
    @location(0) fragNormal: vec3<f32>,
    @location(1) vertexViewSpace: vec3<f32>,
};

struct FragmentOutput {
    @location(0) color: vec4<f32>,
};

@group(1) @binding(0) var texture2D: texture_2d<f32>;
@group(1) @binding(1) var sampler2D: sampler;

@fragment
fn frag_main(input: FragmentInput) -> FragmentOutput {
    let r = reflect(input.vertexViewSpace, input.fragNormal);
    let m = 2.0 * sqrt(pow(r.x, 2.0) + pow(r.y, 2.0) + pow(r.z + 1.0, 2.0));
    let vN = r.xy / m + 0.5;

    let sample = textureSample(texture2D, sampler2D, -vN).rgb;

    var output: FragmentOutput;
    output.color = vec4<f32>(sample, 1.0);

    return output;
}