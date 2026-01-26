<script setup lang="ts">
    import { ref, onMounted } from 'vue'

    const metrics = ref({
    cpu: 0,
    ram: 0,
    status: 0
    })


    onMounted(async () => {
    const res = await fetch('http://backend/metrics')
    metrics.value = await res.json()
    })
</script>

<template>
    <main>
        <h1>System Metrics</h1>
        <p>CPU: {{ metrics.cpu }}%</p>
        <p>RAM: {{ metrics.ram }}%</p>
    </main>
</template>


<style>
main {
font-family: Arial, sans-serif;
padding: 2rem;
}
</style>