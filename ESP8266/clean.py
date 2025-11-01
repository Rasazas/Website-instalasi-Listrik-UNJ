Import("env")
import os
def after_build(source, target, env):
    os.system("pio run --target clean")
env.AddPostAction("buildprog", after_build)