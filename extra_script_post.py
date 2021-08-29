import os
Import("env")

env.Replace(COMPILATIONDB_PATH=os.path.join("$PROJECT_DIR", "compile_commands.json"))

# https://docs.platformio.org/page/faq.html#compilation-database-compile-commands-json
