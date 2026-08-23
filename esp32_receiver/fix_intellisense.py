Import("env")
import os
import json

def fix_intellisense_flags():
    project_dir = env.get("PROJECT_DIR")
    build_dir = env.subst("$BUILD_DIR")
    toolchain_dir = os.path.join(build_dir, "toolchain")
    cflags_path = os.path.join(toolchain_dir, "cflags")
    cflags_clang_path = os.path.join(toolchain_dir, "cflags_clang")
    
    if os.path.exists(cflags_path):
        with open(cflags_path, "r", encoding="utf-8") as f:
            content = f.read()
        # Clang expects -mlong-calls instead of GCC's -mlongcalls
        content_clang = content.replace("-mlongcalls", "-mlong-calls")
        with open(cflags_clang_path, "w", encoding="utf-8") as f:
            f.write(content_clang)

    props_path = os.path.join(project_dir, ".vscode", "c_cpp_properties.json")
    if os.path.exists(props_path):
        try:
            with open(props_path, "r", encoding="utf-8") as f:
                data = json.load(f)
            modified = False
            for cfg in data.get("configurations", []):
                args = cfg.get("compilerArgs", [])
                new_args = []
                for a in args:
                    if a.endswith("cflags"):
                        new_args.append(a + "_clang")
                        modified = True
                    else:
                        new_args.append(a)
                cfg["compilerArgs"] = new_args
            if modified:
                with open(props_path, "w", encoding="utf-8") as f:
                    json.dump(data, f, indent=4)
        except Exception:
            pass

fix_intellisense_flags()
