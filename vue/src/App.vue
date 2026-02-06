<script setup lang="ts">
    import { ref, onMounted, onUnmounted, computed } from 'vue'

    const metrics = ref({
        status: 'disconnected',
        cpu: 0, // %
        cpu_process: 0, // %
        total_ram: 0, // KB -
        used_ram: 0, // KB -
        process_physical_ram: 0, // % -
        total_virtual_ram: 0, // KB -
        used_virtual_ram: 0, // KB -
        process_virtual_ram: 0 // %
    })

    const error = ref('')

    onMounted(() => {
        const es = new EventSource('http://localhost:8080/metrics/stream');
        es.onmessage = (ev) => {
            try {
                metrics.value = JSON.parse(ev.data);
                console.log("Update");
                error.value = '';
            } catch (e) {
                error.value = 'Feil ved parsing av metrics: ' + e;
            }
    };
    es.onerror = (err) => {
        error.value = 'SSE-feil: ' + err;
        es.close();
    };

    onUnmounted(() => {
        es.close();
    });
    });

    const isActive = computed(() => {
        const s = String(metrics.value.status ?? '').toLowerCase()
        return s === 'connected' || s === 'online' || s === 'up' || s === 'true'
    })

    function pct(value: unknown, total: unknown) {
        const v = Number(value) || 0
        const t = Number(total) || 0
        if (!isFinite(v) || !isFinite(t) || t <= 0) return 0
        const p = Math.round((v / t) * 100)
        return Math.min(100, Math.max(0, p))
    }
</script>

<template>
    <div class="container mt-5">
        <header class="mb-4">
            <h1 class="display-5 fw-bold">System Monitor</h1>
            <p class="text-muted">Sanntidsoversikt over systemressurser</p>
        </header>

        <div v-if="error" class="alert alert-danger" role="alert">
            {{ error }}
        </div>

        <div class="row g-4" v-else>
            
            <div class="col-md-4">
                <div class="card h-100 shadow-sm border-0">
                    <div class="card-body text-center">
                        <h5 class="card-title text-muted">CPU Bruk</h5>
                        <h2 class="display-4 fw-bold">{{ metrics.cpu }}%</h2>
                        <div class="progress mt-3">
                            <div class="progress-bar bg-primary" :style="{ width: metrics.cpu + '%' }"></div>
                        </div>
                    </div>
                </div>
            </div>

            <div class="col-md-4">
                <div class="card h-100 shadow-sm border-0">
                    <div class="card-body text-center">
                        <h5 class="card-title text-muted">Total RAM</h5>
                        <h2 class="display-4 fw-bold">{{ metrics.total_ram }} KB</h2>
                        <div class="progress mt-3">
                            <div class="progress-bar bg-info" :style="{ width: '100%' }"></div>
                        </div>
                    </div>
                </div>
            </div>
            <!--Fremføring-->
            <div class="col-md-4">
                <div class="card h-100 shadow-sm border-0">
                    <div class="card-body text-center">
                        <h5 class="card-title text-muted">Used RAM</h5>
                        <h2 class="display-4 fw-bold">{{ metrics.used_ram }} KB</h2>
                        <div class="progress mt-3">
                            <div class="progress-bar bg-info" :style="{ width: pct(metrics.used_ram, metrics.total_ram) + '%' }"></div>
                        </div>
                    </div>
                </div>
            </div>
            <!--Fremføring-->
            <div class="col-md-4">
                <div class="card h-100 shadow-sm border-0">
                    <div class="card-body text-center">
                        <h5 class="card-title text-muted">Physical RAM</h5>
                        <h2 class="display-4 fw-bold">{{ metrics.process_physical_ram }} %</h2>
                        <div class="progress mt-3">
                            <div class="progress-bar bg-info" :style="{ width: metrics.process_physical_ram + '%' }"></div>
                        </div>
                    </div>
                </div>
            </div>
             <div class="col-md-4"> <!--Deler siden inn i kolonner-->
                <div class="card h-100 shadow-sm border-0"> <!--Lager selve "kortet" med en moderne skygge-->
                    <div class="card-body text-center">
                        <h5 class="card-title text-muted">Total Virtual RAM</h5>
                        <h2 class="display-4 fw-bold">{{ metrics.total_virtual_ram }} KB</h2> <!--De henter data direkte fra 
                                                                                              JavaScript-objektet ditt og skriver dem ut-->
                        <div class="progress mt-3">
                            <div class="progress-bar bg-info" :style="{ width: '100%' }"></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="col-md-4">
                <div class="card h-100 shadow-sm border-0">
                    <div class="card-body text-center">
                        <h5 class="card-title text-muted">Used Virtual RAM</h5>
                        <h2 class="display-4 fw-bold">{{ metrics.used_virtual_ram }} KB</h2>
                        <div class="progress mt-3">
                            <div class="progress-bar bg-info" :style="{ width: pct(metrics.used_virtual_ram, metrics.total_virtual_ram) + '%' }"></div>
                        </div>
                    </div>
                </div>
            </div>
            <div class="col-md-4">
                <div class="card h-100 shadow-sm border-0">
                    <div class="card-body text-center">
                        <h5 class="card-title text-muted">Process Virtual RAM</h5>
                        <h2 class="display-4 fw-bold">{{ metrics.process_virtual_ram }} %</h2>
                        <div class="progress mt-3">
                            <div class="progress-bar bg-info" :style="{ width: metrics.process_virtual_ram + '%' }"></div>
                        </div>
                    </div>
                </div>
            </div>
             <div class="col-md-4">
                <div class="card h-100 shadow-sm border-0">
                    <div class="card-body text-center">
                        <h5 class="card-title text-muted">Used and Total Virtual RAM</h5>
                        <h2 class="display-4 fw-bold">{{ Math.round((metrics.used_virtual_ram / metrics.total_virtual_ram * 100)) }} %</h2>
                        <div class="progress mt-3">
                            <div class="progress-bar bg-info" :style="{ width: Math.round((metrics.used_virtual_ram / metrics.total_virtual_ram * 100)) + '%' }"></div>
                        </div>
                    </div>
                </div>
            </div>
            <!-- her skal du pytte ein ny tile som viser prosent regnt ut i %: {{ Math.round((metrics.used_ram / metrics.total_ram * 100)) }} -->
            <div class="col-md-4">
                <div class="card h-100 shadow-sm border-0 bg-dark text-white text-center">
                    <div class="card-body d-flex flex-column justify-content-center">
                        <h5 class="card-title opacity-75">Systemstatus</h5>
                        <div class="d-flex align-items-center justify-content-center mt-2">
                            <span class="status-dot me-2"
                                  :style="{ backgroundColor: isActive ? '#28a745' : '#dc3545', boxShadow: isActive ? '0 0 8px #28a745' : '0 0 8px #dc3545' }"></span>
                            <h3 class="mb-0" :class="isActive ? 'text-success' : 'text-danger'">
                                {{ metrics.status }}
                            </h3>
                        </div>
                    </div>
                </div>
            </div>
            
        </div>
    </div>
</template>

<style scoped>
    /* Bakgrunnsfarge for hele siden */
    :global(body) {
        background-color: #f8f9fa;
    }

    /* En liten animert prikk for statusen */
    .status-dot {
        height: 12px;
        width: 12px;
        background-color: #28a745;
        border-radius: 50%;
        display: inline-block;
        box-shadow: 0 0 8px #28a745;
    }

    /* Gjør kortene litt "løftet" når man holder over dem */
    .card {
        transition: transform 0.2s ease-in-out;
    }
    .card:hover {
        transform: translateY(-5px);
    }
</style>