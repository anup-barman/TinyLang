import React, { useState } from 'react';
import ReactMarkdown from 'react-markdown';


import { docs } from '../data/docsContent';
import { Play, BookOpen } from 'lucide-react';

interface DocsPaneProps {
    onRunSnippet: (code: string) => void;
    isOpen: boolean;
    onClose: () => void;
}

export const DocsPane: React.FC<DocsPaneProps> = ({ onRunSnippet, isOpen, onClose }) => {
    const [activeSection, setActiveSection] = useState<string>(docs[0].id);

    if (!isOpen) return null;

    return (
        <div style={{
            display: 'flex',
            width: '100%',
            background: '#1e1e1e',
            borderLeft: '1px solid #333',
            flexDirection: 'column',
            height: '100%',
        }}>
            {/* Header */}
            <div className="pane-header" style={{ height: '40px', alignItems: 'center', padding: '0 15px', borderBottom: '1px solid #333' }}>
                <span style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#ddd' }}>
                    <BookOpen size={16} /> Documentation
                </span>
                <button onClick={onClose} style={{ background: 'none', border: 'none', color: '#888', cursor: 'pointer', fontSize: '18px' }}>×</button>
            </div>

            <div style={{ flex: 1, display: 'flex', overflow: 'hidden' }}>
                {/* Sidebar Navigation */}
                <div style={{ width: '120px', background: '#181818', overflowY: 'auto', borderRight: '1px solid #333' }}>
                    {docs.map(section => (
                        <div
                            key={section.id}
                            onClick={() => setActiveSection(section.id)}
                            style={{
                                padding: '10px',
                                cursor: 'pointer',
                                fontSize: '13px',
                                color: activeSection === section.id ? '#646cff' : '#888',
                                background: activeSection === section.id ? '#252529' : 'transparent',
                                borderLeft: activeSection === section.id ? '2px solid #646cff' : '2px solid transparent'
                            }}
                        >
                            {section.title}
                        </div>
                    ))}
                </div>

                {/* Content Area */}
                <div style={{ flex: 1, padding: '20px', overflowY: 'auto', color: '#ddd' }}>
                    {docs.map(section => {
                        if (section.id !== activeSection) return null;
                        return (
                            <div key={section.id}>
                                <h2 style={{ marginTop: 0, fontSize: '20px', borderBottom: '1px solid #444', paddingBottom: '10px' }}>{section.title}</h2>
                                <div style={{ fontSize: '14px', lineHeight: '1.6' }}>
                                    <ReactMarkdown>{section.content}</ReactMarkdown>
                                </div>

                                {/* Snippets */}
                                {section.snippets && section.snippets.length > 0 && (
                                    <div style={{ marginTop: '20px' }}>
                                        <h3 style={{ fontSize: '16px', color: '#aaa' }}>Examples</h3>
                                        {section.snippets.map((snippet, idx) => (
                                            <div key={idx} style={{ marginBottom: '20px', background: '#111', borderRadius: '6px', overflow: 'hidden' }}>
                                                <div style={{
                                                    padding: '8px 12px',
                                                    background: '#252529',
                                                    fontSize: '12px',
                                                    color: '#ccc',
                                                    display: 'flex',
                                                    justifyContent: 'space-between',
                                                    alignItems: 'center'
                                                }}>
                                                    <span>{snippet.title}</span>
                                                    <button
                                                        onClick={() => onRunSnippet(snippet.code)}
                                                        className="btn btn-primary"
                                                        style={{ padding: '4px 8px', fontSize: '11px', height: 'auto' }}
                                                    >
                                                        <Play size={10} style={{ marginRight: '4px' }} /> Run
                                                    </button>
                                                </div>
                                                <pre style={{
                                                    margin: 0,
                                                    padding: '12px',
                                                    fontSize: '13px',
                                                    color: '#f8f8f2',
                                                    overflowX: 'auto',
                                                    fontFamily: 'monospace'
                                                }}>
                                                    {snippet.code}
                                                </pre>
                                            </div>
                                        ))}
                                    </div>
                                )}
                            </div>
                        );
                    })}
                </div>
            </div>
        </div>
    );
};
