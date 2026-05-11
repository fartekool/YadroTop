interface ProcessInfo {
    pid: number;
    name: string;
    mem: number;
    status: string;
    threads: number;
}

interface SystemMetrics {
    total: number;
    free: number;
    available: number;
    cpu: number;
    processes: ProcessInfo[];
}

function connect(): void {
    const socket = new WebSocket('ws://localhost:8080/ws');
    const statusEl = document.getElementById('status') as HTMLElement;
    const outputEl = document.getElementById('output') as HTMLElement;
    const processListEl = document.getElementById('process-list') as HTMLElement;

    socket.onopen = () => {
        statusEl.innerText = 'Статус: Онлайн';
        statusEl.style.color = 'green';
    };

    socket.onmessage = (event: MessageEvent) => {
        const data: SystemMetrics = JSON.parse(event.data);
        
        const used = data.total - data.available;
        outputEl.innerHTML = `
            <p><b>Всего:</b> ${data.total.toFixed(1)} MB</p>
            <p><b>Использовано:</b> ${used.toFixed(1)} MB</p>
            <p><b>Доступно:</b> ${data.available.toFixed(1)} MB</p>
            <p><b>Свободно:</b> ${data.free.toFixed(1)} MB</p>
            <p><b>Загрузка:</b> ${data.cpu.toFixed(1)} %</p>
        `;

        if (processListEl) {
            processListEl.innerHTML = data.processes.map(proc => `
                <tr>
                    <td>${proc.pid}</td>
                    <td>${proc.name}</td>
                    <td>${proc.mem.toFixed(2)} MB</td>
                    <td>${proc.status}</td>
                    <td>${proc.threads}</td>
                </tr>
            `).join('');
        }
    };

    socket.onclose = () => {
        statusEl.innerText = 'Статус: Оффлайн. Повтор...';
        statusEl.style.color = 'red';
        setTimeout(connect, 2000);
    };

    socket.onerror = () => {
        socket.close();
    };
}

connect();