import os
import subprocess
import tempfile
import json
import resource
import sys
import logging
from typing import Optional
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI(title="TinyLang Server")

# Enable CORS for frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], # For local dev
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class RunRequest(BaseModel):
    source: str
    stdin: str = ""

class RunResponse(BaseModel):
    success: bool
    compile_errors: list = []
    stdout: str = ""
    stderr: str = ""
    exit_code: int = 0
    time_ms: int = 0
    message: Optional[str] = None

# Configure path to compiler
COMPILER_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../compiler/build/tinylang-compiler"))

def set_limits():
    # Set CPU time limit (seconds)
    resource.setrlimit(resource.RLIMIT_CPU, (3, 3))
    # Set Memory limit (bytes) - 256MB
    resource.setrlimit(resource.RLIMIT_AS, (256 * 1024 * 1024, 256 * 1024 * 1024))

@app.post("/api/run", response_model=RunResponse)
async def run_code(req: RunRequest):
    if not os.path.exists(COMPILER_PATH):
        raise HTTPException(status_code=500, detail="Compiler binary not found")

    # Create temporary source file
    with tempfile.NamedTemporaryFile(mode='w', suffix='.tl', delete=False) as tmp:
        tmp.write(req.source)
        tmp_path = tmp.name

    try:
        # Construct command
        # tinylang-compiler --run --file <path> --stdin <stdin>
        # We need to pass stdin content safe? 
        # The new driver supports --stdin argument directly.
        
        args = [COMPILER_PATH, "--run", "--file", tmp_path, "--stdin", req.stdin]
        
        # Determine if we can use set_limits (Unix only)
        preexec = None
        if sys.platform != "win32":
            preexec = set_limits
            
        proc = subprocess.run(
            args,
            capture_output=True,
            text=True,
            timeout=5,
            preexec_fn=preexec
        )
        
        # The compiler driver outputs JSON to stdout
        # However, if there was a crash or non-JSON output, we handle it
        try:
            output_data = json.loads(proc.stdout)
            return output_data
        except json.JSONDecodeError:
            # Fallback if compiler returned malformed JSON (e.g. segfault printed to stdout not captured by driver, or driver crashed)
            return {
                "success": False,
                "stdout": proc.stdout,
                "stderr": proc.stderr or "Internal Compiler Error (Invalid JSON output)",
                "exit_code": proc.returncode,
                "time_ms": 0,
                "message": "Compiler did not return valid JSON."
            }

    except subprocess.TimeoutExpired:
        return {
            "success": False,
            "stdout": "",
            "stderr": "Execution timed out (5s limit).",
            "exit_code": -1,
            "time_ms": 5000
        }
    except Exception as e:
        return {
            "success": False,
            "stdout": "",
            "stderr": str(e),
            "exit_code": -1,
            "time_ms": 0
        }
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

@app.get("/health")
def health():
    return {"status": "ok", "compiler_found": os.path.exists(COMPILER_PATH)}
