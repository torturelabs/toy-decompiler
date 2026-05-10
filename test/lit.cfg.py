import os
import lit.formats

config.name = "TOY-DECOMPILER"
config.test_format = lit.formats.ShTest(execute_external=True)
config.suffixes = [".mlir"]
config.excludes = [
    "add.s", "add.elf", "add_harness.c",   "add_recovered_harness.c",
    "max.s", "max.elf", "max_harness.c",   "max_recovered_harness.c",
    "pick.s", "pick.elf", "pick_harness.c", "pick_recovered_harness.c",
]

config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.toy_decompiler_obj_root, "test")

tools_dir_lift = os.path.join(config.toy_decompiler_obj_root, "tools", "arm-lift")
tools_dir_opt = os.path.join(config.toy_decompiler_obj_root, "tools", "arm-opt")

config.substitutions.append(("%arm-opt", os.path.join(tools_dir_opt, "arm-opt")))
config.substitutions.append(("%arm-lift", os.path.join(tools_dir_lift, "arm-lift")))
config.substitutions.append(("%FileCheck", config.filecheck))
config.substitutions.append(("%mlir-translate", "mlir-translate"))
config.substitutions.append(("%clang", "clang"))
