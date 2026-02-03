<script setup lang="ts">
    import { ref, onMounted, computed } from 'vue'

    const metrics = ref({
        cpu: 0,
        ram: 0,
        status: 'Aktiv' // Endret til tekst for enkelhets skyld i demoen
    })

    const error = ref('')

    const fetchMetrics = async () => {
        try {
            const res = await fetch('http://localhost:8080/metrics')
            if (!res.ok) throw new Error(`HTTP ${res.status}`)
            const data = await res.json()
            metrics.value = data
            error.value = ''
        } catch (err) {
            console.error(err)
            error.value = 'Kunne ikke hente systemdata fra serveren.'
        }
    }

    onMounted(fetchMetrics)
    const refresh = fetchMetrics

    const isActive = computed(() => {
        const s = String(metrics.value.status ?? '').toLowerCase()
        return s === 'aktiv' || s === 'online' || s === 'up' || s === 'true'
    })
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
                        <h5 class="card-title text-muted">RAM Bruk</h5>
                        <h2 class="display-4 fw-bold">{{ metrics.ram }}%</h2>
                        <div class="progress mt-3">
                            <div class="progress-bar bg-info" :style="{ width: metrics.ram + '%' }"></div>
                        </div>
                    </div>
                </div>
            </div>

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

        <div class="mt-5 border-top pt-4">
            <button class="btn btn-outline-primary shadow-sm" @click="refresh">
                Oppdater manuelt
            </button>
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