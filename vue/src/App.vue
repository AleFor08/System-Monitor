<script setup lang="ts">
    import { ref, onMounted } from 'vue'

    const metrics = ref({
        cpu: 0,
        ram: 0,
        status: 0
    })

    const error = ref('')

    onMounted(async () => {
        try {
            const res = await fetch('http://backend:8080/metrics')
            const data = await res.json()
            metrics.value = data
        } catch (err) {
            error.value = 'Kunne ikke hente metrics: ' + err
        }
    })
</script>

<template>
    <main>
        <h1>System Metrikis</h1>
        <div v-if="error" style="color: red;">
            {{ error }}
        </div>
        <div v-else>
            <p>CPU: {{ metrics.cpu }}%</p>
            <p>RAM: {{ metrics.ram }}%</p>
            
        </div>
        <button class="btn btn-primary">Dette er en Bootstrap-knapp!</button>
    </main>
</template>


<style>
    main {
        font-family: Arial, sans-serif;
        padding: 2rem;
    }
</style>