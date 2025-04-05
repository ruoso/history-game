/**
 * History Game Simulation Visualizer
 * 
 * This application visualizes the JSON logs produced by the History Game simulation.
 * It allows interactive replay of the simulation with controls for playback speed,
 * pause/play, step forward/backward, and visual representation of NPCs, objects,
 * perceptions, and actions.
 */

// Constants
const WORLD_SIZE = 1000; // Match the size used in the simulation

const COLORS = {
    NPC: '#3498db',
    FOOD: '#2ecc71',
    SHELTER: '#9b59b6',
    PERCEPTION: '#f39c12',
    ACTION: '#e74c3c',
    BACKGROUND: '#f8f9fa'
};

const DRIVE_COLORS = {
    'Belonging': '#ff7675',
    'Grief': '#2d3436',
    'Curiosity': '#fdcb6e',
    'Sustenance': '#55efc4',
    'Shelter': '#a29bfe',
    'Pride': '#e17055'
};

// Main Application
class SimulationVisualizer {
    constructor() {
        this.initializeElements();
        this.initializeEventListeners();
        this.resetState();
    }
    
    initializeElements() {
        // Controls
        this.loadButton = document.getElementById('loadButton');
        this.playPauseButton = document.getElementById('playPauseButton');
        this.stepBackButton = document.getElementById('stepBackButton');
        this.stepForwardButton = document.getElementById('stepForwardButton');
        this.fileInput = document.getElementById('logFile');
        this.playbackSpeed = document.getElementById('playbackSpeed');
        this.speedDisplay = document.getElementById('speedDisplay');
        this.timelineSlider = document.getElementById('timelineSlider');
        
        // Info displays
        this.currentTickDisplay = document.getElementById('currentTick');
        this.currentGenerationDisplay = document.getElementById('currentGeneration');
        this.npcCountDisplay = document.getElementById('npcCount');
        this.objectCountDisplay = document.getElementById('objectCount');
        this.totalTicksDisplay = document.getElementById('totalTicks');
        
        // Entity details and event log
        this.entityDetailsElement = document.getElementById('entity-details');
        this.eventLogElement = document.getElementById('event-log');
        
        // Canvas
        this.canvas = document.getElementById('worldCanvas');
        this.ctx = this.canvas.getContext('2d');
        this.resizeCanvas();
    }
    
    initializeEventListeners() {
        // Load simulation button
        this.loadButton.addEventListener('click', () => this.loadSimulation());
        
        // Playback controls
        this.playPauseButton.addEventListener('click', () => this.togglePlayPause());
        this.stepBackButton.addEventListener('click', () => this.stepBack());
        this.stepForwardButton.addEventListener('click', () => this.stepForward());
        
        // Playback speed
        this.playbackSpeed.addEventListener('input', () => {
            const speed = parseFloat(this.playbackSpeed.value);
            this.speedDisplay.textContent = `${speed.toFixed(1)}x`;
            this.simulation.playbackSpeed = speed;
        });
        
        // Timeline slider
        this.timelineSlider.addEventListener('input', () => {
            if (this.simulation && this.simulation.events) {
                const tickIndex = Math.floor(this.timelineSlider.value / 100 * this.simulation.events.length);
                this.goToTick(tickIndex);
            }
        });
        
        // Canvas click for entity selection
        this.canvas.addEventListener('click', (e) => this.handleCanvasClick(e));
        
        // Window resize
        window.addEventListener('resize', () => this.resizeCanvas());
    }
    
    resetState() {
        this.simulation = {
            events: null,
            entities: new Map(),
            objects: new Map(),
            currentTickIndex: 0,
            isPlaying: false,
            playbackSpeed: 1,
            animationFrameId: null,
            lastFrameTime: 0
        };
        
        // Reset UI
        this.currentTickDisplay.textContent = '-';
        this.currentGenerationDisplay.textContent = '-';
        this.npcCountDisplay.textContent = '-';
        this.objectCountDisplay.textContent = '-';
        this.totalTicksDisplay.textContent = '-';
        this.entityDetailsElement.innerHTML = '<p>Click on an entity to see details</p>';
        this.eventLogElement.innerHTML = '<p>Simulation events will appear here</p>';
        
        // Disable controls
        this.setControlsEnabled(false);
        
        // Clear canvas
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        this.ctx.fillStyle = COLORS.BACKGROUND;
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        this.ctx.fillStyle = '#333';
        this.ctx.font = '16px sans-serif';
        this.ctx.textAlign = 'center';
        this.ctx.fillText('Load a simulation file to begin', this.canvas.width / 2, this.canvas.height / 2);
    }
    
    setControlsEnabled(enabled) {
        this.playPauseButton.disabled = !enabled;
        this.stepBackButton.disabled = !enabled;
        this.stepForwardButton.disabled = !enabled;
        this.playbackSpeed.disabled = !enabled;
        this.timelineSlider.disabled = !enabled;
    }
    
    resizeCanvas() {
        const container = this.canvas.parentElement;
        this.canvas.width = container.clientWidth - 40; // Account for padding
        this.canvas.height = container.clientHeight - 40;
        
        // Redraw if we have data
        if (this.simulation && this.simulation.events) {
            this.renderCurrentState();
        }
    }
    
    async loadSimulation() {
        if (!this.fileInput.files || !this.fileInput.files[0]) {
            alert('Please select a simulation log file');
            return;
        }
        
        try {
            const file = this.fileInput.files[0];
            const fileContents = await file.text();
            const simulationData = JSON.parse(fileContents);
            
            if (!Array.isArray(simulationData) || simulationData.length === 0) {
                throw new Error('Invalid simulation data: Expected a non-empty array');
            }
            
            // Reset and initialize with new data
            this.resetState();
            this.simulation.events = simulationData;
            
            // Find start and end events to determine simulation parameters
            const startEvent = simulationData.find(event => event.type === 'SIMULATION_START');
            const endEvent = simulationData.find(event => event.type === 'SIMULATION_END');
            
            if (startEvent) {
                this.npcCountDisplay.textContent = startEvent.npc_count || '?';
                this.objectCountDisplay.textContent = startEvent.object_count || '?';
            }
            
            if (endEvent) {
                this.totalTicksDisplay.textContent = endEvent.total_ticks || simulationData.length;
            } else {
                this.totalTicksDisplay.textContent = simulationData.length;
            }
            
            // Enable controls
            this.setControlsEnabled(true);
            
            // Set timeline max value
            this.timelineSlider.max = simulationData.length - 1;
            this.timelineSlider.value = 0;
            
            // Go to the first tick
            this.goToTick(0);
            
            console.log('Simulation loaded successfully', {
                events: simulationData.length,
                startEvent,
                endEvent
            });
        } catch (error) {
            console.error('Error loading simulation:', error);
            alert(`Error loading simulation: ${error.message}`);
        }
    }
    
    togglePlayPause() {
        if (!this.simulation.events) return;
        
        this.simulation.isPlaying = !this.simulation.isPlaying;
        this.playPauseButton.textContent = this.simulation.isPlaying ? 'Pause' : 'Play';
        
        if (this.simulation.isPlaying) {
            this.simulation.lastFrameTime = performance.now();
            this.animateSimulation();
        } else {
            cancelAnimationFrame(this.simulation.animationFrameId);
        }
    }
    
    stepBack() {
        if (!this.simulation.events) return;
        
        const newIndex = Math.max(0, this.simulation.currentTickIndex - 1);
        this.goToTick(newIndex);
    }
    
    stepForward() {
        if (!this.simulation.events) return;
        
        const newIndex = Math.min(this.simulation.events.length - 1, this.simulation.currentTickIndex + 1);
        this.goToTick(newIndex);
    }
    
    goToTick(tickIndex) {
        if (!this.simulation.events || tickIndex < 0 || tickIndex >= this.simulation.events.length) return;
        
        this.simulation.currentTickIndex = tickIndex;
        this.timelineSlider.value = (tickIndex / (this.simulation.events.length - 1)) * 100;
        
        // Process the event at this tick
        this.processEvent(this.simulation.events[tickIndex]);
        
        // Update displays
        this.updateDisplays();
        
        // Render the state
        this.renderCurrentState();
    }
    
    animateSimulation() {
        if (!this.simulation.isPlaying) return;
        
        const now = performance.now();
        const deltaTime = now - this.simulation.lastFrameTime;
        const tickDuration = 1000 / this.simulation.playbackSpeed; // ms per tick
        
        if (deltaTime >= tickDuration) {
            this.simulation.lastFrameTime = now;
            
            // Advance to next tick
            if (this.simulation.currentTickIndex < this.simulation.events.length - 1) {
                this.stepForward();
            } else {
                // End of simulation
                this.simulation.isPlaying = false;
                this.playPauseButton.textContent = 'Play';
                return;
            }
        }
        
        this.simulation.animationFrameId = requestAnimationFrame(() => this.animateSimulation());
    }
    
    processEvent(event) {
        // Update event log
        this.addEventToLog(event);
        
        // Process based on event type
        switch (event.type) {
            case 'TICK_START':
                this.currentTickDisplay.textContent = event.tick_number;
                this.currentGenerationDisplay.textContent = event.generation;
                break;
                
            case 'TICK_END':
                this.currentTickDisplay.textContent = event.tick_number;
                this.currentGenerationDisplay.textContent = event.generation;
                this.npcCountDisplay.textContent = event.npc_count;
                this.objectCountDisplay.textContent = event.object_count;
                break;
                
            case 'SIMULATION_START':
                this.npcCountDisplay.textContent = event.npc_count;
                this.objectCountDisplay.textContent = event.object_count;
                break;
                
            case 'SIMULATION_END':
                this.npcCountDisplay.textContent = event.npc_count;
                this.objectCountDisplay.textContent = event.object_count;
                this.totalTicksDisplay.textContent = event.total_ticks;
                break;
                
            case 'PERCEPTION':
                // Highlight perception in visualization
                this.highlightPerception(event);
                break;
                
            case 'ACTION_EXECUTION':
                // Highlight action in visualization
                this.highlightAction(event);
                break;
                
            case 'DRIVE_UPDATE':
                // Update NPC drives
                this.updateDrives(event);
                break;
        }
    }
    
    highlightPerception(event) {
        // Implementation will depend on the event structure
        // This is a placeholder for perception visualization
    }
    
    highlightAction(event) {
        // Implementation will depend on the event structure
        // This is a placeholder for action visualization
    }
    
    updateDrives(event) {
        // Implementation will depend on the event structure
        // This is a placeholder for drive updates
    }
    
    renderCurrentState() {
        // Clear canvas
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        this.ctx.fillStyle = COLORS.BACKGROUND;
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        // Draw grid lines
        this.drawGrid();
        
        // Draw entities from our state
        this.drawEntities();
    }
    
    drawGrid() {
        const gridSize = 50;
        const width = this.canvas.width;
        const height = this.canvas.height;
        
        this.ctx.strokeStyle = '#ddd';
        this.ctx.lineWidth = 1;
        
        // Draw vertical lines
        for (let x = gridSize; x < width; x += gridSize) {
            this.ctx.beginPath();
            this.ctx.moveTo(x, 0);
            this.ctx.lineTo(x, height);
            this.ctx.stroke();
        }
        
        // Draw horizontal lines
        for (let y = gridSize; y < height; y += gridSize) {
            this.ctx.beginPath();
            this.ctx.moveTo(0, y);
            this.ctx.lineTo(width, y);
            this.ctx.stroke();
        }
    }
    
    drawEntities() {
        // Get the canvas dimensions for scaling
        const canvasWidth = this.canvas.width;
        const canvasHeight = this.canvas.height;
        
        // Calculate scale factors (we'll scale the world to fit the canvas)
        const scaleX = canvasWidth / WORLD_SIZE;
        const scaleY = canvasHeight / WORLD_SIZE;
        
        // Extract entities from the simulation state
        const entities = [];
        
        // Collect all NPCs and objects from our state
        if (this.simulation && this.simulation.events) {
            // Find the latest entity updates from the current tick
            const currentTickIndex = this.simulation.currentTickIndex;
            const maxTickToConsider = Math.min(currentTickIndex, this.simulation.events.length - 1);
            
            // We'll track the latest update for each entity ID
            const entityUpdates = new Map();
            
            // Process all events up to the current tick
            for (let i = 0; i <= maxTickToConsider; i++) {
                const event = this.simulation.events[i];
                if (event.type === 'ENTITY_UPDATE') {
                    // Store the latest update for each entity
                    entityUpdates.set(event.entity_id, event);
                }
            }
            
            // Convert the entity updates to renderable entities
            for (const event of entityUpdates.values()) {
                // Make sure we have required fields
                if (!event.position || !event.entity_type) {
                    continue;
                }
                
                // Create an entity object for rendering
                entities.push({
                    id: event.entity_id,
                    type: event.entity_type,
                    x: event.position.x * scaleX,
                    y: event.position.y * scaleY,
                    action: event.current_action,
                    drives: event.drives
                });
            }
        }
        
        // If no entities found, show some placeholder entities
        if (entities.length === 0) {
            const placeholders = [
                { type: 'NPC', id: 'npc_1', x: 100, y: 100 },
                { type: 'NPC', id: 'npc_2', x: 200, y: 150 },
                { type: 'Object', id: 'food_1', x: 150, y: 250 },
                { type: 'Object', id: 'shelter_1', x: 300, y: 200 }
            ];
            entities.push(...placeholders);
        }
        
        // Find action events for the current tick to highlight actions
        const activeActions = new Map();
        if (this.simulation && this.simulation.events && this.simulation.currentTickIndex < this.simulation.events.length) {
            // Get events for the current tick
            const currentTickEvents = this.simulation.events[this.simulation.currentTickIndex];
            
            // Check if it's an action event
            if (currentTickEvents.type === 'ACTION_EXECUTION') {
                activeActions.set(currentTickEvents.entity_id, {
                    action: currentTickEvents.action_type,
                    target: currentTickEvents.target_id
                });
            }
        }
        
        // Draw all entities
        entities.forEach(entity => {
            let color;
            let size = 12;
            let shape = 'circle'; // Default shape
            
            // Determine color and shape based on entity type
            if (entity.type === 'NPC') {
                color = COLORS.NPC;
                shape = 'circle';
                size = 15;
            } else if (entity.id.includes('food')) {
                color = COLORS.FOOD;
                shape = 'square';
            } else if (entity.id.includes('shelter')) {
                color = COLORS.SHELTER;
                shape = 'triangle';
            } else {
                color = '#999';
                shape = 'square';
            }
            
            // Draw entity based on its shape
            this.ctx.fillStyle = color;
            
            if (shape === 'circle') {
                this.ctx.beginPath();
                this.ctx.arc(entity.x, entity.y, size, 0, Math.PI * 2);
                this.ctx.fill();
            } else if (shape === 'square') {
                this.ctx.fillRect(entity.x - size, entity.y - size, size * 2, size * 2);
            } else if (shape === 'triangle') {
                this.ctx.beginPath();
                this.ctx.moveTo(entity.x, entity.y - size);
                this.ctx.lineTo(entity.x + size, entity.y + size);
                this.ctx.lineTo(entity.x - size, entity.y + size);
                this.ctx.closePath();
                this.ctx.fill();
            }
            
            // Draw action indicator if this entity is performing an action
            if (activeActions.has(entity.id)) {
                const actionInfo = activeActions.get(entity.id);
                this.ctx.strokeStyle = COLORS.ACTION;
                this.ctx.lineWidth = 2;
                this.ctx.beginPath();
                this.ctx.arc(entity.x, entity.y, size + 5, 0, Math.PI * 2);
                this.ctx.stroke();
                
                // If action has a target, draw a line to it
                if (actionInfo.target) {
                    const targetEntity = entities.find(e => e.id === actionInfo.target);
                    if (targetEntity) {
                        this.ctx.beginPath();
                        this.ctx.moveTo(entity.x, entity.y);
                        this.ctx.lineTo(targetEntity.x, targetEntity.y);
                        this.ctx.stroke();
                    }
                }
            }
            
            // Draw current action text if available
            if (entity.action) {
                this.ctx.fillStyle = '#333';
                this.ctx.font = '10px sans-serif';
                this.ctx.textAlign = 'center';
                this.ctx.fillText(entity.action, entity.x, entity.y - size - 5);
            }
            
            // Draw entity ID label
            this.ctx.fillStyle = '#333';
            this.ctx.font = '12px sans-serif';
            this.ctx.textAlign = 'center';
            this.ctx.fillText(entity.id, entity.x, entity.y + size + 15);
        });
    }
    
    handleCanvasClick(event) {
        // Get canvas-relative coordinates
        const rect = this.canvas.getBoundingClientRect();
        const x = event.clientX - rect.left;
        const y = event.clientY - rect.top;
        
        // Find if any entity was clicked
        if (!this.simulation || !this.simulation.events) {
            return;
        }
        
        // Get all entities from the current state
        const currentTickIndex = this.simulation.currentTickIndex;
        const entityUpdates = new Map();
        
        // Process all events up to the current tick to get the latest state
        for (let i = 0; i <= currentTickIndex; i++) {
            const event = this.simulation.events[i];
            if (event.type === 'ENTITY_UPDATE') {
                entityUpdates.set(event.entity_id, event);
            }
        }
        
        // Scale factors for canvas to world conversion
        const scaleX = this.canvas.width / WORLD_SIZE;
        const scaleY = this.canvas.height / WORLD_SIZE;
        
        // Find the clicked entity
        let clickedEntity = null;
        let closestDistance = 20; // Detection radius, in pixels
        
        for (const event of entityUpdates.values()) {
            if (!event.position) continue;
            
            // Convert world coords to canvas coords
            const entityX = event.position.x * scaleX;
            const entityY = event.position.y * scaleY;
            
            // Calculate distance from click to entity
            const distance = Math.sqrt(
                Math.pow(entityX - x, 2) + 
                Math.pow(entityY - y, 2)
            );
            
            // If this is the closest entity within our threshold, select it
            if (distance < closestDistance) {
                closestDistance = distance;
                clickedEntity = event;
            }
        }
        
        // If we found an entity, show its details
        if (clickedEntity) {
            const worldX = clickedEntity.position.x;
            const worldY = clickedEntity.position.y;
            
            this.showEntityDetails({ 
                id: clickedEntity.entity_id,
                type: clickedEntity.entity_type,
                position: { x: worldX, y: worldY },
                drives: clickedEntity.drives,
                currentAction: clickedEntity.current_action
            });
        }
    }
    
    showEntityDetails(entity) {
        let detailsHTML = `
            <h4>${entity.id}</h4>
            <div>Type: ${entity.type}</div>
            <div>Position: (${entity.position.x.toFixed(1)}, ${entity.position.y.toFixed(1)})</div>
        `;
        
        if (entity.currentAction) {
            detailsHTML += `<div>Action: ${entity.currentAction}</div>`;
        }
        
        if (entity.drives && entity.drives.length > 0) {
            detailsHTML += '<div class="drives-container">';
            entity.drives.forEach(drive => {
                const percent = Math.round(drive.value * 100);
                detailsHTML += `
                    <div class="drive-bar">
                        <div class="drive-fill" style="width: ${percent}%; background-color: ${DRIVE_COLORS[drive.type] || '#999'}"></div>
                        <span class="drive-name">${drive.type}: ${percent}%</span>
                    </div>
                `;
            });
            detailsHTML += '</div>';
        }
        
        this.entityDetailsElement.innerHTML = detailsHTML;
    }
    
    addEventToLog(event) {
        const logEntry = document.createElement('p');
        
        // Format the event message based on type
        let message = `Tick ${event.tick_number || ''}: ${event.type}`;
        
        switch (event.type) {
            case 'PERCEPTION':
                message = `${event.perceiver_id} perceived ${event.perceived_id}`;
                break;
                
            case 'ACTION_EXECUTION':
                message = `${event.entity_id} performed ${event.action_type}`;
                if (event.targets && event.targets.length > 0) {
                    message += ` targeting ${event.targets[0]}`;
                }
                break;
                
            case 'DRIVE_UPDATE':
                message = `${event.npc_id} drive update`;
                break;
        }
        
        logEntry.textContent = message;
        
        // Add to log with newest at the top
        this.eventLogElement.insertBefore(logEntry, this.eventLogElement.firstChild);
        
        // Limit log entries to avoid overwhelming the DOM
        while (this.eventLogElement.children.length > 100) {
            this.eventLogElement.removeChild(this.eventLogElement.lastChild);
        }
    }
    
    updateDisplays() {
        // This method updates all display information based on the current state
        // Most updates are handled in processEvent(), but this ensures consistency
    }
}

// Initialize when the DOM is fully loaded
document.addEventListener('DOMContentLoaded', () => {
    // Create and start the visualizer
    window.visualizer = new SimulationVisualizer();
});