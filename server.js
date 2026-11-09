// ============================================================================
// Hospital Emergency Management System - Localhost Web Server
// Serves the Web Dashboard (ui/) with zero external dependencies.
// ============================================================================

const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 3000;
const UI_DIR = path.join(__dirname, 'ui');
const ROOT_DIR = __dirname;

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

const server = http.createServer((req, res) => {
    // Normalize URL and remove query strings
    let reqUrl = req.url.split('?')[0];
    if (reqUrl === '/') reqUrl = '/index.html';

    // Allow serving root-level text files (e.g. er_patient_log.txt, daily_er_report.txt)
    let filePath;
    if (reqUrl === '/er_patient_log.txt' || reqUrl === '/daily_er_report.txt') {
        filePath = path.join(ROOT_DIR, reqUrl);
    } else {
        filePath = path.join(UI_DIR, reqUrl);
    }

    // Security check: prevent directory traversal
    if (!filePath.startsWith(UI_DIR) && !filePath.startsWith(ROOT_DIR)) {
        res.writeHead(403, { 'Content-Type': 'text/plain' });
        res.end('403 Forbidden');
        return;
    }

    fs.stat(filePath, (err, stats) => {
        if (err || !stats.isFile()) {
            res.writeHead(404, { 'Content-Type': 'text/plain' });
            res.end('404 Not Found');
            return;
        }

        const ext = path.extname(filePath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';

        res.writeHead(200, {
            'Content-Type': contentType,
            'Access-Control-Allow-Origin': '*'
        });

        const stream = fs.createReadStream(filePath);
        stream.pipe(res);
    });
});

server.listen(PORT, '127.0.0.1', () => {
    console.log('======================================================================');
    console.log('    HOSPITAL EMERGENCY MANAGEMENT SYSTEM - LOCAL WEB SERVER           ');
    console.log('======================================================================');
    console.log(`  Local URL : http://localhost:${PORT}`);
    console.log(`  IP URL    : http://127.0.0.1:${PORT}`);
    console.log(`  Serving   : ${UI_DIR}`);
    console.log('======================================================================');
    console.log('Press Ctrl+C to stop the server.');
});

