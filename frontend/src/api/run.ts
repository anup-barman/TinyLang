export interface RunResponse {
    success: boolean;
    compile_errors: Array<{ phase: string; message: string; line: number; col: number }>;
    stdout: string;
    stderr: string;
    exit_code: number;
    time_ms: number;
    message?: string;
}

export const runCode = async (source: string, stdin: string): Promise<RunResponse> => {
    const response = await fetch('http://localhost:8000/api/run', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({ source, stdin }),
    });

    if (!response.ok) {
        throw new Error(`Server fetch failed: ${response.statusText}`);
    }

    return response.json();
};
