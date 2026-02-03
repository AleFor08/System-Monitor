<script setup lang="ts">
    import { ref, onMounted, onUnmounted } from 'vue'

    const metrics = ref({
        cpu: 0,
        ram: 0,
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
</script>

<template>
    <main>
        <h1>System Metrics</h1>
        <div v-if="error" style="color: red;">
            {{ error }}
        </div>
        <div v-else>
            <p>CPU: {{ metrics.cpu }}%</p>
            <p>RAM: {{ metrics.ram }}%</p>
        </div>
    </main>
</template>

<style>
    main {
        font-family: Arial, sans-serif;
        padding: 2rem;
    }
</style>