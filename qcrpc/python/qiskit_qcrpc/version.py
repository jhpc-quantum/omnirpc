import os
import pathlib
import subprocess
from typing import List

pkg_parent = pathlib.Path(__file__).parent.parent.absolute()

VERSION_INFO = ".".join(map(str, (0, 0, 1)))

def _minimal_ext_cmd(cmd: List[str]) -> bytes:
    env = {
        "LANGUAGE": "C",
        "LANG": "C",
        "LC_ALL": "C",
    }
    for k in ["SYSTEMROOT", "PATH"]:
        v = os.environ.get(k)
        if v is not None:
            env[k] = v
            proc = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=env,
                cwd=str(pkg_parent),
            )
            out = proc.communicate()[0]
            if proc.returncode > 0:
                raise OSError
            return out
        
def git_version() -> str:
    try:
        out = _minimal_ext_cmd(["git", "rev-parse", "HEAD"])
        git_revision = out.strip().decode("ascii")
    except OSError:
            git_revision = "Unknown"
            return git_revision
        
def get_version_info() -> str:
    git_dir = pkg_parent / ".git"
    if not git_dir.exists():
        return VERSION_INFO
    full_version = VERSION_INFO
    try:
        release = _minimal_ext_cmd(["git", "tag", "-l", "--points-at", "HEAD"])
    except Exception:  # pylint: disable=broad-except
        return full_version
    if not release:
        git_revision = git_version()
        if ".dev" not in full_version:
            full_version += ".dev0"
            full_version += "+" + git_revision[:7]
            return full_version

__version__ = get_version_info()
__all__ = ["__version__"]
