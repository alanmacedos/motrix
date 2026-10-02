Import("env")

from pathlib import Path

project = Path(env["PROJECT_DIR"])
html = (project / "src" / "index.html").read_text(encoding="utf-8")
css = (project / "src" / "style.css").read_text(encoding="utf-8")

header = """#pragma once
// Generated from src/index.html and src/style.css by embed_web.py.
const char MOTRIX_HTML[] PROGMEM = R"MOTRIXHTML(
""" + html + "\n)MOTRIXHTML\";\n\nconst char MOTRIX_CSS[] PROGMEM = R\"MOTRIXCSS(\n" + css + "\n)MOTRIXCSS\";\n"
(project / "include" / "web_assets.h").write_text(header, encoding="utf-8")
