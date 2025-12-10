import React, { useState, useRef } from 'react';
import Split from 'react-split';
import Editor, { OnMount } from '@monaco-editor/react';
import { Play, Square, Book, Code2, AlertCircle } from 'lucide-react';
import { runCode, RunResponse } from '../api/run';
import { DocsPane } from './DocsPane';

interface EditorLayoutProps {
    initialCode?: string;
}

const EXAMPLES = [
    {
        id: 'factorial',
        label: 'Factorial',
        code: `// Welcome to TinyLang!
// Try the factorial example:

func factorial(int n) -> int {
  if (n < 2) {
    return 1;
  }
  return n * factorial(n - 1);
}

func main() {
  let n = 5;
  print(factorial(n));
}`
    },
    {
        id: 'arithmetic',
        label: 'Arithmetic',
        code: `func main() { print(1+2*3); }`
    },
    {
        id: 'loop',
        label: 'Loop',
        code: `func main() {
  for(let i=0; i<5; i=i+1) {
    print(i);
  }
}`
    }
];

export const EditorLayout: React.FC<EditorLayoutProps> = () => {
    const [source, setSource] = useState(EXAMPLES[0].code);
    const [stdin, setStdin] = useState('');
    const [output, setOutput] = useState('');
    const [isRunning, setIsRunning] = useState(false);
    const [status, setStatus] = useState('Ready');
    const [errors, setErrors] = useState<RunResponse['compile_errors']>([]);
    const [activeTab, setActiveTab] = useState<'output' | 'status'>('output');
    const [showDocs, setShowDocs] = useState(false);

    const editorRef = useRef<any>(null);
    const monacoRef = useRef<any>(null);

    const handleEditorDidMount: OnMount = (editor, monaco) => {
        editorRef.current = editor;
        monacoRef.current = monaco;

        editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => {
            handleRun();
        });
    };

    const handleRun = async () => {
        // ... (keep implementation)
        if (isRunning) return;
        setIsRunning(true);
        setStatus('Compiling & Running...');
        setErrors([]);
        setOutput('');

        // Clear markers
        if (monacoRef.current) {
            monacoRef.current.editor.setModelMarkers(editorRef.current.getModel(), 'owner', []);
        }

        try {
            const res = await runCode(source, stdin);

            if (res.success) {
                setOutput(res.stdout);
                setStatus(`Finished in ${res.time_ms}ms (Exit: ${res.exit_code})`);
                setActiveTab('output');
            } else {
                setOutput(res.stdout + '\n' + res.stderr);
                setStatus(`Failed (Exit: ${res.exit_code})`);
                setErrors(res.compile_errors);
                setActiveTab('status');

                // Add markers
                if (monacoRef.current && res.compile_errors.length > 0) {
                    const markers = res.compile_errors.map(err => ({
                        startLineNumber: err.line || 1,
                        startColumn: err.col || 1,
                        endLineNumber: err.line || 1,
                        endColumn: (err.col || 1) + 10,
                        message: `${err.phase}: ${err.message}`,
                        severity: monacoRef.current.MarkerSeverity.Error,
                    }));
                    monacoRef.current.editor.setModelMarkers(editorRef.current.getModel(), 'owner', markers);
                }
            }
        } catch (err: any) {
            setOutput(`Error: ${err.message}`);
            setStatus('Network Error');
        } finally {
            setIsRunning(false);
        }
    };

    const handleSnippetRun = (code: string) => {
        setSource(code);
    };

    const renderEditorArea = () => (
        <Split
            sizes={[60, 40]}
            minSize={100}
            expandToMin={false}
            gutterSize={10}
            gutterAlign="center"
            dragInterval={1}
            direction="horizontal"
            className="split-flex"
            style={{ display: 'flex', flex: 1, overflow: 'hidden' }}
        >
            {/* Editor Pane */}
            <div style={{ overflow: 'hidden', display: 'flex', flexDirection: 'column' }}>
                <Editor
                    height="100%"
                    defaultLanguage="cpp"
                    theme="vs-dark"
                    value={source}
                    onChange={(val) => setSource(val || '')}
                    onMount={handleEditorDidMount}
                    options={{
                        minimap: { enabled: false },
                        fontSize: 14,
                        fontFamily: "'JetBrains Mono', 'Fira Code', Consolas, monospace",
                        scrollBeyondLastLine: false,
                        automaticLayout: true,
                        padding: { top: 20 }
                    }}
                />
            </div>

            {/* IO Pane */}
            <div style={{ display: 'flex', flexDirection: 'column' }}>
                <Split
                    sizes={[30, 70]}
                    direction="vertical"
                    className="split-flex-v"
                    style={{ display: 'flex', flexDirection: 'column', flex: 1 }}
                >
                    <div style={{ display: 'flex', flexDirection: 'column', background: '#1e1e1e' }}>
                        <div className="pane-header">STDIN (Input)</div>
                        <textarea
                            value={stdin}
                            onChange={(e) => setStdin(e.target.value)}
                            style={{ flex: 1, background: '#1e1e1e', color: '#ddd', border: 'none', resize: 'none', padding: '10px', fontFamily: 'monospace', outline: 'none' }}
                            placeholder="Input for your program..."
                        />
                    </div>
                    <div style={{ display: 'flex', flexDirection: 'column', background: '#1e1e1e' }}>
                        {/* Tabs Header */}
                        <div style={{ display: 'flex', background: '#252526', borderBottom: '1px solid #333' }}>
                            <button
                                onClick={() => setActiveTab('output')}
                                style={{
                                    background: activeTab === 'output' ? '#1e1e1e' : 'transparent',
                                    color: activeTab === 'output' ? '#fff' : '#aaa',
                                    border: 'none',
                                    borderTop: activeTab === 'output' ? '2px solid #646cff' : '2px solid transparent',
                                    padding: '8px 16px',
                                    cursor: 'pointer',
                                    fontSize: '12px',
                                    fontWeight: 'bold',
                                    outline: 'none'
                                }}
                            >
                                OUTPUT
                            </button>
                            <button
                                onClick={() => setActiveTab('status')}
                                style={{
                                    background: activeTab === 'status' ? '#1e1e1e' : 'transparent',
                                    color: activeTab === 'status' ? (errors.length ? '#ff6b6b' : '#fff') : (errors.length ? '#ff6b6b' : '#aaa'),
                                    border: 'none',
                                    borderTop: activeTab === 'status' ? '2px solid #646cff' : '2px solid transparent',
                                    padding: '8px 16px',
                                    cursor: 'pointer',
                                    fontSize: '12px',
                                    fontWeight: 'bold',
                                    outline: 'none',
                                    display: 'flex',
                                    gap: '6px',
                                    alignItems: 'center'
                                }}
                            >
                                STATUS {errors.length > 0 && <span style={{ background: '#ff6b6b', color: '#1e1e1e', borderRadius: '50%', width: '16px', height: '16px', display: 'flex', alignItems: 'center', justifyContent: 'center', fontSize: '10px' }}>{errors.length}</span>}
                            </button>
                        </div>

                        {/* Tab Content */}
                        <div style={{
                            flex: 1,
                            padding: '10px',
                            whiteSpace: 'pre-wrap',
                            fontFamily: 'monospace',
                            overflow: 'auto',
                            color: '#ddd',
                            background: '#151515'
                        }}>
                            {activeTab === 'output' ? (
                                output || <span style={{ color: '#444' }}>// Output will appear here...</span>
                            ) : (
                                <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
                                    <div style={{ color: status.includes('Failed') ? '#ff6b6b' : '#4caf50', fontWeight: 'bold' }}>
                                        {status}
                                    </div>
                                    {errors.length > 0 && (
                                        <div style={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
                                            {errors.map((err, i) => (
                                                <div key={i} style={{ background: '#2d2d2d', padding: '8px', borderLeft: '3px solid #ff6b6b' }}>
                                                    <div style={{ fontWeight: 'bold', color: '#ff6b6b' }}>{err.phase} Error</div>
                                                    <div>{err.message}</div>
                                                    <div style={{ opacity: 0.6, fontSize: '0.9em' }}>Line {err.line}, Column {err.col}</div>
                                                </div>
                                            ))}
                                        </div>
                                    )}
                                    {errors.length === 0 && !status.includes('Failed') && (
                                        <div style={{ color: '#aaa' }}>No errors found.</div>
                                    )}
                                </div>
                            )}
                        </div>
                    </div>
                </Split>
            </div>
        </Split>
    );

    return (
        <div style={{ flex: 1, display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>
            {/* Toolbar */}
            <div style={{ height: '50px', background: '#1a1a1a', borderBottom: '1px solid #333', display: 'flex', alignItems: 'center', padding: '0 20px', justifyContent: 'space-between' }}>
                {/* Left: Branding */}
                <div style={{ display: 'flex', alignItems: 'center', gap: '8px', fontWeight: 'bold', fontSize: '18px', color: '#646cff', width: '200px' }}>
                    <Code2 /> TinyLang
                </div>

                {/* Center: Run Controls */}
                <div style={{ display: 'flex', gap: '10px', alignItems: 'center' }}>
                    <button
                        className="btn btn-primary"
                        onClick={handleRun}
                        disabled={isRunning}
                        style={{ height: '36px', padding: '0 24px', fontSize: '14px', fontWeight: 'bold' }}
                    >
                        {isRunning ? <Square size={16} fill="white" /> : <Play size={16} fill="white" />}
                        {isRunning ? 'Running...' : 'Run'}
                    </button>
                </div>

                {/* Right: Docs Toggle */}
                < div style={{ width: '200px', display: 'flex', justifyContent: 'flex-end' }}>
                    <button
                        className="btn"
                        onClick={() => setShowDocs(!showDocs)}
                        style={{ background: showDocs ? '#252529' : 'transparent', color: showDocs ? '#fff' : '#aaa' }}
                    >
                        <Book size={18} /> Documentation
                    </button>
                </div >
            </div >

            {/* Main Content Area */}
            {/* If docs are shown, we split the main area horizontally. If not, just show editor layout. */}
            <div style={{ flex: 1, display: 'flex', overflow: 'hidden' }}>
                {showDocs ? (
                    <Split
                        sizes={[70, 30]}
                        minSize={300}
                        expandToMin={false}
                        gutterSize={10}
                        gutterAlign="center"
                        snapOffset={30}
                        dragInterval={1}
                        direction="horizontal"
                        className="split-flex"
                        style={{ display: 'flex', flex: 1, overflow: 'hidden' }}
                    >
                        {/* Left Side: Editor + IO */}
                        <div style={{ flex: 1, display: 'flex', overflow: 'hidden' }}>
                            {renderEditorArea()}
                        </div>

                        {/* Right Side: Docs */}
                        <DocsPane
                            isOpen={true}
                            onClose={() => setShowDocs(false)}
                            onRunSnippet={handleSnippetRun}
                        />
                    </Split>
                ) : (
                    <div style={{ flex: 1, display: 'flex', overflow: 'hidden' }}>
                        {renderEditorArea()}
                    </div>
                )}
            </div>

            {/* Footer Status Bar */}
            <div className="status-bar">
                <span style={{ display: 'flex', alignItems: 'center', gap: '5px' }}>
                    {status.includes('Failed') ? <AlertCircle size={12} /> : null}
                    {status}
                </span>
                <span style={{ fontFamily: 'monospace', opacity: 0.6 }}>v3.0.0</span>
            </div>
        </div >
    );
};
