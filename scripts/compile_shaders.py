import subprocess
import os

class ShaderCompiler:
    _shaders_path = "../src/engine/shaders/glsl"
    _output_path = "../src/engine/shaders/";
    _glslc = "glslc"
    _shader = ""

    def __init__(self, shader_file):
        self._shader = shader_file

    def compile(self):
        if not os.path.exists(self._shader):
            print(f"Error: {self._shader} does not exist.")
            return
        
        

def main():
    # get list of files in directopry
    for file in os.listdir(os.path.relpath("../src/engine/shaders/glsl/")):
        if file.endswith(".frag") or file.endswith(".vert"):
            # input
            input_shader = os.path.abspath("../src/engine/shaders/glsl/" + file)

            # output
            extension = file.split(".")[1]
            out_shader = file.replace(extension, "spv")
            out_shader = out_shader.replace(".spv", "_" + extension + ".spv")
            out_shader = os.path.abspath("../src/engine/shaders/glsl/bytecode/" + out_shader)
            
            result = subprocess.run(["glslc", input_shader, "-g", "-o", out_shader],
                           stdout = subprocess.PIPE)
            print(result.stdout)
    
if __name__ == "__main__":
    main();
