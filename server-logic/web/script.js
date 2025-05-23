// Fonction principale pour récupérer et mettre à jour les données
async function updateData() {
    try {
        const response = await fetch('/api/data');
        const data = await response.json();
        
        // Mise à jour des valeurs actuelles avec préservation du style
        const tempElement = document.getElementById('current-temp');
        const humElement = document.getElementById('current-hum');
        const timeElement = document.getElementById('last-update-time');
        const statusElement = document.getElementById('status');
        
        if (tempElement && data.current && data.current.temperature !== undefined) {
            tempElement.innerHTML = `${data.current.temperature.toFixed(1)}<span class="unit">°C</span>`;
        }
        
        if (humElement && data.current && data.current.humidity !== undefined) {
            humElement.innerHTML = `${data.current.humidity.toFixed(1)}<span class="unit">%</span>`;
        }
        
        // Mise à jour du statut avec style préservé
        if (statusElement) {
            statusElement.innerHTML = '<span class="status-dot"></span>Système Actif';
            statusElement.style.background = 'linear-gradient(135deg, #48bb78, #38a169)';
            statusElement.style.boxShadow = '0 4px 15px rgba(72, 187, 120, 0.4)';
        }
        
        // Mise à jour de l'heure
        if (timeElement) {
            timeElement.textContent = new Date().toLocaleTimeString('fr-FR', {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit'
            });
        }
        
        // Mise à jour des statistiques si disponibles
        if (data.stats) {
            updateStatistics(data.stats);
        }
        
    } catch (error) {
        console.error('Erreur lors de la récupération des données:', error);
        
        // Gestion des erreurs avec style préservé
        const statusElement = document.getElementById('status');
        
        if (statusElement) {
            statusElement.innerHTML = '<span class="status-dot" style="background: #f56565; animation: none;"></span>Système Inactif';
            statusElement.style.background = 'linear-gradient(135deg, #f56565, #e53e3e)';
            statusElement.style.boxShadow = '0 4px 15px rgba(245, 101, 101, 0.4)';
        }
        
        // Afficher des valeurs d'erreur
        const tempElement = document.getElementById('current-temp');
        const humElement = document.getElementById('current-hum');
        
        if (tempElement) {
            tempElement.innerHTML = '<span class="unit">--°C</span>';
        }
        if (humElement) {
            humElement.innerHTML = '<span class="unit">--%</span>';
        }
    }
}

// Fonction pour mettre à jour les statistiques
function updateStatistics(stats) {
    const statsContainer = document.getElementById('stats');
    if (!statsContainer || !stats) return;
    
    const statItems = [
        { value: stats.avgTemp?.toFixed(1) || '--', label: 'Temp. Moyenne', unit: '°C' },
        { value: stats.avgHumidity?.toFixed(1) || '--', label: 'Hum. Moyenne', unit: '%' },
        { value: stats.maxTemp?.toFixed(1) || '--', label: 'Temp. Max', unit: '°C' },
        { value: stats.minHumidity?.toFixed(1) || '--', label: 'Hum. Min', unit: '%' }
    ];
    
    statsContainer.innerHTML = statItems.map(item => `
        <div class="stat-item">
            <div class="stat-value">${item.value}${item.unit}</div>
            <div class="stat-label">${item.label}</div>
        </div>
    `).join('');
}

// Fonction de simulation des données
function simulateData() {
    const temp = (20 + Math.random() * 10).toFixed(1);
    const humidity = (45 + Math.random() * 30).toFixed(1);
    
    const tempElement = document.getElementById('current-temp');
    const humElement = document.getElementById('current-hum');
    
    if (tempElement) {
        tempElement.innerHTML = `${temp}<span class="unit">°C</span>`;
    }
    if (humElement) {
        humElement.innerHTML = `${humidity}<span class="unit">%</span>`;
    }
}

// Initialisation
document.addEventListener('DOMContentLoaded', function() {
    // Animation d'entrée des cartes
    const cards = document.querySelectorAll('.card');
    cards.forEach((card, index) => {
        card.style.animationDelay = `${index * 0.2}s`;
    });
    
    // Première tentative de récupération des données
    updateData();
    
    // Si l'API n'est pas disponible, on utilise des données simulées
    setTimeout(() => {
        // Connexion perdue, mode simulation
        const lastUpdate = document.getElementById('last-update');
        if (lastUpdate) {
            lastUpdate.innerHTML = 'Mode Simulation - Données non disponibles';
        }

        const tempElement = document.getElementById('current-temp');
        if (tempElement && tempElement.textContent.includes('--')) {
            console.log('API non disponible, utilisation de la simulation');
            
            simulateData();
            setInterval(simulateData, 10000);
        }
    }, 1000);
});

// Mise à jour toutes les 10 secondes
setInterval(updateData, 10000);