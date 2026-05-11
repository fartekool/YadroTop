var socket;
function connect() {
    socket = new WebSocket('ws://localhost:8080/ws');
    var statusEl = document.getElementById('status');
    var outputEl = document.getElementById('output');
    var processListEl = document.getElementById('process-list');
    socket.onopen = function () {
        statusEl.innerText = 'Статус: Онлайн';
        statusEl.style.color = 'green';
    };
    socket.onmessage = function (event) {
        var data = JSON.parse(event.data);
        var used = data.total - data.available;
        outputEl.innerHTML = "\n            <p><b>\u0412\u0441\u0435\u0433\u043E:</b> ".concat(data.total.toFixed(1), " MB</p>\n            <p><b>\u0418\u0441\u043F\u043E\u043B\u044C\u0437\u043E\u0432\u0430\u043D\u043E:</b> ").concat(used.toFixed(1), " MB</p>\n            <p><b>\u0414\u043E\u0441\u0442\u0443\u043F\u043D\u043E:</b> ").concat(data.available.toFixed(1), " MB</p>\n            <p><b>\u0421\u0432\u043E\u0431\u043E\u0434\u043D\u043E:</b> ").concat(data.free.toFixed(1), " MB</p>\n            <p><b>\u0417\u0430\u0433\u0440\u0443\u0437\u043A\u0430:</b> ").concat(data.cpu.toFixed(1), " %</p>\n        ");
        if (processListEl) {
            processListEl.innerHTML = data.processes.map(function (proc) { return "\n                <tr>\n                    <td>".concat(proc.pid, "</td>\n                    <td>").concat(proc.name, "</td>\n                    <td>").concat(proc.user, "</td>\n                    <td>").concat(proc.proc_cpu.toFixed(2), " %</td>\n                    <td>").concat(proc.mem.toFixed(2), " MB</td>\n                    <td>").concat(proc.threads, "</td>\n                    <td>").concat(proc.status, "</td>\n                </tr>\n            "); }).join('');
        }
    };
    socket.onclose = function () {
        statusEl.innerText = 'Статус: Оффлайн. Повтор...';
        statusEl.style.color = 'red';
        setTimeout(connect, 2000);
    };
    socket.onerror = function () {
        socket.close();
    };
}
window.changeSort = function (type) {
    if (socket && socket.readyState === WebSocket.OPEN) {
        socket.send("sort_".concat(type));
    }
};
connect();
