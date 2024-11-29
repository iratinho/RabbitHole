struct FragmentInput {
    @location(0) nearPoint: vec3<f32>,
    @location(1) farPoint: vec3<f32>,
}

struct FragmentOutput {
    @builtin(frag_depth) depth: f32,
    @location(0) color: vec4<f32>,
};

struct SceneData {
    view: mat4x4<f32>,
    proj: mat4x4<f32>,
};

@group(0) @binding(0) var<uniform> sceneData: SceneData;

fn DrawGrid(position: vec3<f32>, scale: f32, intensity: f32, bSkip: bool) -> vec4<f32> {
    let coord = position.xz * scale;
    let derivative = fwidth(coord); 
    let grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    let line = 1.0 - smoothstep(0.0, 1.0, min(grid.x, grid.y));
    let minY = min(derivative.y, 1.0);
    let minX = max(derivative.x, 1.0);
    var color = vec4<f32>(0.2, 0.2, 0.2, 1.0) * line;

    if (bSkip) {
        color.a = 0.0;
    }

    return color * intensity;
}

fn DrawAxis(position: vec3<f32>) -> vec4<f32> {
    let coord = position.xz * 1.3;
    let derivaitve = fwidth(coord);
    let grid = abs(fract(coord - 0.5) - 0.5) / derivaitve;
    let minimumz = min(derivaitve.y, 1.0);
    let minimumx = min(derivaitve.x, 1.0);
    var color = vec4<f32>(0);

    if(position.x > -1.0 * minimumx && position.x < 1.0 * minimumx) {
        color = vec4<f32>(41, 128, 185, 255) / 255.0;
        color.a = 1.0;
    }

    if(position.z > -1.0 * minimumz && position.z < 1.0 * minimumz) {
        color = vec4<f32>(231, 76, 60, 255) / 255.0;
        color.a = 1.0;
    }

    return color;
}

fn ComputeDepth(fragProj: mat4x4<f32>, fragView: mat4x4<f32>, position: vec4<f32>) -> f32 {
    let clipSpacePos = fragProj * fragView * position;
    return (clipSpacePos.z / clipSpacePos.w); // use w prespective divide to get depth in NDC
}

fn ComputeLinearDepth(fragProj: mat4x4<f32>, fragView: mat4x4<f32>, position: vec4<f32>) -> f32 {
    let clipSpacePos = fragProj * fragView * position;
    let clipSpaceDepth = (clipSpacePos.z / clipSpacePos.w) * 2.0 - 1.0; // put back between -1 and 1 (NDC
    let linearDepth = (2.0 * 0.01 * 1000) / (1000 + 0.01 - clipSpaceDepth * (1000 - 0.01)); // linearize depth
    return linearDepth / 1000.0; // normalize between 0 and 1
}

@fragment
fn frag_main(input: FragmentInput) -> FragmentOutput {
    var output: FragmentOutput;

    let cutoff = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);
    let position = input.nearPoint + cutoff * (input.farPoint - input.nearPoint);
    
    output.depth = ComputeDepth(sceneData.proj, sceneData.view, vec4<f32>(position, 1.0));

    let linearDepth = ComputeLinearDepth(sceneData.proj, sceneData.view, vec4<f32>(position, 1.0));
    let fading = exp(-linearDepth * 70.0);

    let axisColor = DrawAxis(position);
    let bFragmentAxis = axisColor.a >= 1.0;

    output.color = DrawGrid(position, 1.0, 0.5, bFragmentAxis);
    output.color += DrawGrid(position, (1.0 / 10.0), 0.96, bFragmentAxis);
    output.color.a *= fading * 0.8;

    if(bFragmentAxis) {
        output.color = axisColor;
        output.color.a = fading;
    }


    output.color *= f32(cutoff > 0.0);

    // if(output.color.a < 0.01) {
    //     discard;
    // }

    // output.color.a = fading * 0.8;

    // output.color *= f32(cutoff > 0.0);
    // output.color = vec4<f32>(linearDepth * 100, 0, 0, 1.0);

    return output;

    // todo need to discard elements that are on the depth buffer so that we dont draw on top of meshes
}