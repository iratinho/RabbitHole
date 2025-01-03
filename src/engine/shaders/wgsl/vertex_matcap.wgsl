struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) normal: vec3<f32>,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) fragNormal: vec3<f32>,
    @location(1) vertexViewSpace: vec3<f32>,
};

struct ModelData {
    modelViewMatrix: mat4x4<f32>,
    mvpMatrix: mat4x4<f32>,
    normalMatrix: mat4x4<f32>,
};

@group(0) @binding(0) var<uniform> modelData: ModelData;

@vertex
fn vert_main(vertexInput: VertexInput) -> VertexOutput {
    var output: VertexOutput;

    output.vertexViewSpace = normalize((modelData.modelViewMatrix * vec4<f32>(vertexInput.position, 1.0)).xyz);
    output.fragNormal = normalize((modelData.normalMatrix * vec4<f32>(vertexInput.normal, 0.0)).xyz);
    output.position = modelData.mvpMatrix * vec4<f32>(vertexInput.position, 1.0);

    return output;
}