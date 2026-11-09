// ============================================================================
// Hospital Emergency Management System - Local Web Server
// Serves the Web Dashboard with zero external dependencies.
// ============================================================================

const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 3000;
const ROOT_DIR = path.resolve(__dirname, '..');
const PUBLIC_DIR = path.join(ROOT_DIR, 'public');
const UI_DIR = path.join(ROOT_DIR, 'ui');

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.txt': 'text/plain; charset=utf-8',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.ico': 'image/x-icon',
    '.svg': 'image/svg+xml'
};

const requestHandler = (req, res) => {
    let reqUrl = (req.url || '/').split('?')[0];
    if (reqUrl === '/' || reqUrl === '') reqUrl = '/index.html';

    // Remove leading /ui/ if requested
    let cleanUrl = reqUrl.replace(/^\/ui(\/|$)/, '/');
    if (cleanUrl === '/' || cleanUrl === '') cleanUrl = '/index.html';

    // Check public/ directory first, then root, then ui/
    let filePath = path.join(PUBLIC_DIR, cleanUrl);
    if (!fs.existsSync(filePath) || fs.statSync(filePath).isDirectory()) {
        filePath = path.join(ROOT_DIR, cleanUrl);
    }
    if (!fs.existsSync(filePath) || fs.statSync(filePath).isDirectory()) {
        filePath = path.join(UI_DIR, cleanUrl);
    }

    // Security check: prevent directory traversal
    const resolvedPath = path.resolve(filePath);
    if (!resolvedPath.startsWith(ROOT_DIR)) {
        res.writeHead(403, { 'Content-Type': 'text/plain' });
        res.end('403 Forbidden');
        return;
    }

    fs.stat(resolvedPath, (err, stats) => {
        if (err || !stats.isFile()) {
            const fallbackPath = path.join(PUBLIC_DIR, 'index.html');
            if (fs.existsSync(fallbackPath)) {
                res.writeHead(200, {
                    'Content-Type': 'text/html; charset=utf-8',
                    'Access-Control-Allow-Origin': '*'
                });
                fs.createReadStream(fallbackPath).pipe(res);
                return;
            }
            res.writeHead(404, { 'Content-Type': 'text/plain' });
            res.end('404 Not Found');
            return;
        }

        const ext = path.extname(resolvedPath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';

        res.writeHead(200, {
            'Content-Type': contentType,
            'Access-Control-Allow-Origin': '*'
        });

        const stream = fs.createReadStream(resolvedPath);
        stream.pipe(res);
    });
};

const server = http.createServer(requestHandler);
server.listen(PORT, '127.0.0.1', () => {
    console.log('======================================================================');
    console.log('    HOSPITAL EMERGENCY MANAGEMENT SYSTEM - LOCAL WEB SERVER           ');
    console.log('======================================================================');
    console.log(`  Local URL : http://localhost:${PORT}`);
    console.log(`  IP URL    : http://127.0.0.1:${PORT}`);
    console.log('======================================================================');
    console.log('Press Ctrl+C to stop the server.');
});

module.exports = server;
