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
        this.stepForwardButton = document.getElementById('stepForwardButton');
        this.fileInput = document.getElementById('logFile');
        this.playbackSpeed = document.getElementById('playbackSpeed');
        this.speedDisplay = document.getElementById('speedDisplay');
        
        // Info displays
        this.currentTickDisplay = document.getElementById('currentTick');
        this.currentGenerationDisplay = document.getElementById('currentGeneration');
        this.npcCountDisplay = document.getElementById('npcCount');
        this.objectCountDisplay = document.getElementById('objectCount');
        this.totalTicksDisplay = document.getElementById('totalTicks');
        this.zoomDisplay = document.getElementById('zoomDisplay');
        
        // Entity details
        this.entityDetailsElement = document.getElementById('entity-details');
        
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
        this.stepForwardButton.addEventListener('click', () => this.stepForward());
        
        // Playback speed (ticks per second)
        this.playbackSpeed.addEventListener('input', () => {
            const ticksPerSecond = parseInt(this.playbackSpeed.value);
            this.speedDisplay.textContent = ticksPerSecond;
            this.simulation.ticksPerSecond = ticksPerSecond;
        });
        
        // Canvas click for entity selection
        this.canvas.addEventListener('click', (e) => {
            if (!this.simulation.isPanning) {
                this.handleCanvasClick(e);
            }
        });
        
        // Mouse wheel for zooming
        this.canvas.addEventListener('wheel', (e) => {
            e.preventDefault();
            this.handleZoom(e);
        });
        
        // Mouse down for panning
        this.canvas.addEventListener('mousedown', (e) => {
            if (e.button === 0) { // Left button
                this.startPan(e);
            }
        });
        
        // Mouse move for panning
        document.addEventListener('mousemove', (e) => {
            if (this.simulation.isPanning) {
                this.continuePan(e);
            }
        });
        
        // Mouse up to end panning
        document.addEventListener('mouseup', (e) => {
            if (e.button === 0) { // Left button
                this.endPan();
            }
        });
        
        // Mouse leave to end panning
        this.canvas.addEventListener('mouseleave', () => {
            this.endPan();
        });
        
        // Toggle control panel
        const toggleButton = document.getElementById('togglePanel');
        if (toggleButton) {
            toggleButton.addEventListener('click', () => {
                const panel = document.querySelector('.control-panel');
                const panelContent = document.querySelector('.panel-content');
                
                if (panelContent.style.display === 'none') {
                    panelContent.style.display = 'flex';
                    toggleButton.textContent = '-';
                } else {
                    panelContent.style.display = 'none';
                    toggleButton.textContent = '+';
                }
            });
        }
        
        // Window resize
        window.addEventListener('resize', () => this.resizeCanvas());
    }
    
    // Handle zoom with mouse wheel
    handleZoom(event) {
        if (!this.simulation.events) return;
        
        const delta = -Math.sign(event.deltaY) * 0.1;
        const newZoom = Math.max(0.1, Math.min(3.0, this.simulation.zoomLevel + delta));
        
        // Get mouse position relative to canvas
        const rect = this.canvas.getBoundingClientRect();
        const mouseX = event.clientX - rect.left;
        const mouseY = event.clientY - rect.top;
        
        // Calculate old world coordinates at mouse position
        const oldZoom = this.simulation.zoomLevel;
        const oldWorldX = this.simulation.viewportX + mouseX / oldZoom;
        const oldWorldY = this.simulation.viewportY + mouseY / oldZoom;
        
        // Calculate new viewport position to keep mouse over same world point
        const newViewportX = oldWorldX - mouseX / newZoom;
        const newViewportY = oldWorldY - mouseY / newZoom;
        
        // Update simulation state
        this.simulation.zoomLevel = newZoom;
        this.simulation.viewportX = newViewportX;
        this.simulation.viewportY = newViewportY;
        
        // Update zoom display
        this.zoomDisplay.textContent = `${Math.round(newZoom * 100)}%`;
        
        // Render with new viewport
        this.renderCurrentState();
    }
    
    // Start panning
    startPan(event) {
        if (!this.simulation.events) return;
        
        this.simulation.isPanning = true;
        this.simulation.lastPanPoint = {
            x: event.clientX,
            y: event.clientY
        };
        
        // Change cursor to indicate panning
        this.canvas.style.cursor = 'grabbing';
    }
    
    // Continue panning as mouse moves
    continuePan(event) {
        if (!this.simulation.isPanning) return;
        
        const dx = event.clientX - this.simulation.lastPanPoint.x;
        const dy = event.clientY - this.simulation.lastPanPoint.y;
        
        // Update viewport based on mouse movement and zoom level
        this.simulation.viewportX -= dx / this.simulation.zoomLevel;
        this.simulation.viewportY -= dy / this.simulation.zoomLevel;
        
        // Update last point
        this.simulation.lastPanPoint = {
            x: event.clientX,
            y: event.clientY
        };
        
        // Render with new viewport
        this.renderCurrentState();
    }
    
    // End panning
    endPan() {
        if (this.simulation.isPanning) {
            this.simulation.isPanning = false;
            this.canvas.style.cursor = 'default';
        }
    }
    
    resetState() {
        this.simulation = {
            events: null,
            entities: new Map(), // Store the latest state of each entity
            currentEventIndex: 0,
            currentTickNumber: 0,
            isPlaying: false,
            ticksPerSecond: 5, // Default: 5 ticks per second
            animationFrameId: null,
            lastFrameTime: 0,
            activeActions: new Map(), // Store active actions for visualization
            viewportX: 0, // X offset for viewport (for panning)
            viewportY: 0, // Y offset for viewport (for panning)
            zoomLevel: 1.0, // Zoom level (1.0 = 100%)
            isPanning: false, // Whether user is currently panning
            lastPanPoint: null, // Last point during panning
            worldSize: WORLD_SIZE // Default world size, may be overridden
        };
        
        // Reset UI
        this.currentTickDisplay.textContent = '-';
        this.currentGenerationDisplay.textContent = '-';
        this.npcCountDisplay.textContent = '-';
        this.objectCountDisplay.textContent = '-';
        this.totalTicksDisplay.textContent = '-';
        this.entityDetailsElement.innerHTML = '<p>Click on an entity to see details</p>';
        
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
        this.stepForwardButton.disabled = !enabled;
        this.playbackSpeed.disabled = !enabled;
    }
    
    resizeCanvas() {
        // Make the canvas fill the entire window
        this.canvas.width = window.innerWidth;
        this.canvas.height = window.innerHeight;
        
        console.log(`Resized canvas to ${this.canvas.width}x${this.canvas.height}`);
        
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
            
            // Store events and prepare entity map for faster lookups
            this.simulation.events = simulationData;
            this.simulation.currentEventIndex = 0;
            this.simulation.currentTickNumber = 0;
            this.simulation.activeActions = new Map();
            
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
            
            console.log('Simulation loaded successfully', {
                events: simulationData.length,
                startEvent,
                endEvent
            });
            
            // Validate that the first event is SIMULATION_START as expected
            if (simulationData.length === 0) {
                throw new Error("Simulation data is empty");
            }
            
            const firstEvent = simulationData[0];
            if (firstEvent.type !== 'SIMULATION_START') {
                console.warn("First event is not SIMULATION_START:", firstEvent);
                throw new Error("Invalid simulation data: First event must be SIMULATION_START");
            }
            
            // Process just the first event (SIMULATION_START) to initialize entities and world
            console.log("Processing initial SIMULATION_START event:", firstEvent);
            this.processEvent(firstEvent);
            
            // Set initial index to 1 since we've processed the first event
            this.simulation.currentEventIndex = 1;
            
            // Process the first tick starting with the second event
            this.processNextTick();
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
    
    stepForward() {
        if (!this.simulation.events) return;
        
        // Process the next full tick
        this.processNextTick();
    }
    
    processNextEvent() {
        if (!this.simulation.events || 
            this.simulation.currentEventIndex >= this.simulation.events.length) {
            // End of simulation
            this.simulation.isPlaying = false;
            this.playPauseButton.textContent = 'Play';
            return false;
        }
        
        // Get the current event
        const event = this.simulation.events[this.simulation.currentEventIndex];
        
        // Process the event
        console.log(`Processing event ${this.simulation.currentEventIndex}:`, event.type);
        this.processEvent(event);
        
        // Move to the next event
        this.simulation.currentEventIndex++;
        
        return true;
    }
    
    processNextTick() {
        let processedAnyEvents = false;
        let tickEnd = false;
        
        // Process events until we find a TICK_END or run out of events
        while (!tickEnd) {
            const success = this.processNextEvent();
            
            if (!success) {
                break; // No more events to process
            }
            
            processedAnyEvents = true;
            
            // Get the event we just processed (which is now the previous event)
            const eventIndex = this.simulation.currentEventIndex - 1;
            const event = this.simulation.events[eventIndex];
            
            // Check if it was a TICK_END event
            if (event.type === 'TICK_END') {
                tickEnd = true;
                console.log(`Completed tick ${event.tick_number}`);
            }
        }
        
        return processedAnyEvents;
    }
    
    animateSimulation() {
        if (!this.simulation.isPlaying) return;
        
        const now = performance.now();
        const deltaTime = now - this.simulation.lastFrameTime;
        const tickDuration = 1000 / this.simulation.ticksPerSecond; // ms per tick based on ticks/second
        
        if (deltaTime >= tickDuration) {
            this.simulation.lastFrameTime = now;
            
            // Process the next complete tick
            const tickProcessed = this.processNextTick();
            
            // If no tick was processed, we've reached the end
            if (!tickProcessed || !this.simulation.isPlaying) {
                return;
            }
            
            console.log(`Tick ${this.simulation.currentTickNumber} completed. Waiting ${tickDuration}ms for next tick.`);
        }
        
        // Calculate time before next tick
        const timeToNextTick = Math.max(0, tickDuration - (performance.now() - this.simulation.lastFrameTime));
        console.log(`Time to next tick: ${timeToNextTick.toFixed(1)}ms`);
        
        this.simulation.animationFrameId = requestAnimationFrame(() => this.animateSimulation());
    }
    
    processEvent(event) {
        // Log all events for debugging
        console.log("Processing event:", event.type, event);
        
        // Process based on event type
        switch (event.type) {
            case 'TICK_START':
                this.simulation.currentTickNumber = event.tick_number;
                this.currentTickDisplay.textContent = event.tick_number;
                this.currentGenerationDisplay.textContent = event.generation;
                
                // Clear action flags at the start of a new tick
                this.simulation.activeActions.clear();
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
                
                // If world size is provided, update the constant
                if (event.world_size) {
                    console.log(`Setting world size to ${event.world_size}x${event.world_size}`);
                    // We can't modify the constant directly, but we can use this for calculations
                    this.simulation.worldSize = event.world_size;
                }
                
                // If initial entities are provided, set up entity state
                if (event.entities && Array.isArray(event.entities)) {
                    console.log(`Initializing ${event.entities.length} entities from SIMULATION_START`);
                    console.log("First entity:", JSON.stringify(event.entities[0], null, 2));
                    
                    // Clear existing entities
                    this.simulation.entities.clear();
                    
                    // Add each entity to our state
                    for (const entity of event.entities) {
                        if (entity.id && entity.position) {
                            // Format the entity data consistently for our rendering system
                            const entityData = {
                                entity_id: entity.id, // Use id from data
                                entity_type: entity.type || (entity.id.includes('npc') ? 'NPC' : 'Object'),
                                position: {
                                    x: entity.position.x,
                                    y: entity.position.y
                                },
                                current_action: entity.current_action,
                                drives: entity.drives
                            };
                            
                            console.log(`Added entity ${entity.id} at position (${entity.position.x}, ${entity.position.y})`);
                            
                            // Store entity in our map
                            this.simulation.entities.set(entity.id, entityData);
                        } else {
                            console.warn("Entity missing required fields:", entity);
                        }
                    }
                    
                    console.log(`Initialized ${this.simulation.entities.size} entities`);
                    
                    // Render the initial state
                    this.renderCurrentState();
                }
                break;
                
            case 'SIMULATION_END':
                this.npcCountDisplay.textContent = event.npc_count;
                this.objectCountDisplay.textContent = event.object_count;
                this.totalTicksDisplay.textContent = event.total_ticks;
                break;
                
            case 'ENTITY_UPDATE':
                // Inspect the event thoroughly
                console.log("ENTITY_UPDATE event structure:", JSON.stringify(event, null, 2));
                
                // Store the entity update in our entities map
                if (event.entity_id && event.position) {
                    console.log("Updating entity:", event.entity_id, "at position", event.position);
                    
                    // Make sure to store a properly formatted entity object
                    const entityData = {
                        entity_id: event.entity_id,
                        entity_type: event.entity_type || 'Unknown',
                        position: event.position,
                        current_action: event.current_action,
                        drives: event.drives
                    };
                    
                    // Store in our entity map
                    this.simulation.entities.set(event.entity_id, entityData);
                    console.log("Entity map size after update:", this.simulation.entities.size);
                    
                    // Dump the first few entities for debugging
                    if (this.simulation.entities.size < 5) {
                        console.log("Current entities:", 
                            Array.from(this.simulation.entities.entries())
                                .map(([id, data]) => `${id}: (${data.position.x}, ${data.position.y})`).join(', '));
                    }
                } else {
                    console.error("Invalid ENTITY_UPDATE event:", event);
                }
                
                // Render after each entity update
                this.renderCurrentState();
                break;
                
            case 'ACTION_EXECUTION':
                // Store action for visualization
                console.log("Recording action:", event.entity_id, event.action_type, event.target_id);
                this.simulation.activeActions.set(event.entity_id, {
                    action: event.action_type,
                    target: event.target_id
                });
                
                // Render after recording action
                this.renderCurrentState();
                break;
                
            case 'PERCEPTION':
                // Currently not visualized
                break;
                
            case 'DRIVE_UPDATE':
                // Currently not visualized
                break;
                
            default:
                console.log("Unknown event type:", event.type);
                break;
        }
    }
    
    // We've replaced these methods with direct state updates in processEvent
    // Actions and perceptions are now tracked in simulation.activeActions
    
    renderCurrentState() {
        console.log("Rendering state with", this.simulation.entities.size, "entities");
        
        // Clear canvas
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
        this.ctx.fillStyle = COLORS.BACKGROUND;
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        // Draw grid lines
        this.drawGrid();
        
        // Draw entities from our state
        this.drawEntities();
        
        // Log entity positions
        console.log("Active actions:", Array.from(this.simulation.activeActions.entries()));
        
        // Force a redraw
        this.canvas.style.display = 'none';
        this.canvas.offsetHeight; // Force reflow
        this.canvas.style.display = 'block';
    }
    
    drawGrid() {
        const width = this.canvas.width;
        const height = this.canvas.height;
        
        // Apply zoom and pan to the grid
        const zoomLevel = this.simulation.zoomLevel || 1.0;
        const viewportX = this.simulation.viewportX || 0;
        const viewportY = this.simulation.viewportY || 0;
        
        // Use the world size from the SIMULATION_START event if available
        const worldSize = this.simulation.worldSize || WORLD_SIZE;
        
        // Calculate grid spacing based on zoom level
        const baseGridSize = 100; // Grid cell size in world units
        const scaledGridSize = baseGridSize * (width / worldSize) * zoomLevel;
        
        // Calculate offset for panning
        const offsetX = (viewportX * width / worldSize * zoomLevel) % scaledGridSize;
        const offsetY = (viewportY * height / worldSize * zoomLevel) % scaledGridSize;
        
        // Draw a slightly lighter background
        this.ctx.fillStyle = '#f8f9fa';
        this.ctx.fillRect(0, 0, width, height);
        
        // Set grid line style
        this.ctx.strokeStyle = '#ddd';
        this.ctx.lineWidth = 1;
        this.ctx.font = '10px Arial';
        this.ctx.fillStyle = '#999';
        this.ctx.textAlign = 'left';
        
        // Calculate world coordinates of grid lines
        const firstXLine = Math.floor(viewportX / baseGridSize) * baseGridSize;
        const firstYLine = Math.floor(viewportY / baseGridSize) * baseGridSize;
        
        // Draw vertical lines and coordinates
        for (let x = -offsetX; x < width; x += scaledGridSize) {
            this.ctx.beginPath();
            this.ctx.moveTo(x, 0);
            this.ctx.lineTo(x, height);
            this.ctx.stroke();
            
            // Calculate actual world coordinate
            const worldX = firstXLine + (x + offsetX) / scaledGridSize * baseGridSize;
            
            // Draw coordinate label
            this.ctx.fillText(worldX.toFixed(0), x + 5, 15);
        }
        
        // Draw horizontal lines and coordinates
        for (let y = -offsetY; y < height; y += scaledGridSize) {
            this.ctx.beginPath();
            this.ctx.moveTo(0, y);
            this.ctx.lineTo(width, y);
            this.ctx.stroke();
            
            // Calculate actual world coordinate
            const worldY = firstYLine + (y + offsetY) / scaledGridSize * baseGridSize;
            
            // Draw coordinate label
            this.ctx.fillText(worldY.toFixed(0), 5, y + 15);
        }
        
        // Draw world origin for reference if in view
        const originX = (-viewportX) * (width / worldSize) * zoomLevel;
        const originY = (-viewportY) * (height / worldSize) * zoomLevel;
        
        if (originX >= 0 && originX <= width && originY >= 0 && originY <= height) {
            this.ctx.fillStyle = '#ff0000';
            this.ctx.beginPath();
            this.ctx.arc(originX, originY, 5, 0, Math.PI * 2);
            this.ctx.fill();
            this.ctx.fillText('(0,0)', originX + 8, originY - 8);
        }
    }
    
    drawEntities() {
        // Get the canvas dimensions for scaling
        const canvasWidth = this.canvas.width;
        const canvasHeight = this.canvas.height;
        
        // Apply zoom and pan to the scale factors
        const zoomLevel = this.simulation.zoomLevel || 1.0;
        const viewportX = this.simulation.viewportX || 0;
        const viewportY = this.simulation.viewportY || 0;
        
        // Use the world size from the SIMULATION_START event if available
        const worldSize = this.simulation.worldSize || WORLD_SIZE;
        
        // Calculate scale factors with zoom
        const scaleX = (canvasWidth / worldSize) * zoomLevel;
        const scaleY = (canvasHeight / worldSize) * zoomLevel;
        
        console.log(`Canvas size: ${canvasWidth}x${canvasHeight}, Scale: ${scaleX}x${scaleY}, Zoom: ${zoomLevel}x`);
        
        // Extract entities from the simulation state
        const entities = [];
        
        // Collect all NPCs and objects from our stored entity state
        if (this.simulation && this.simulation.entities.size > 0) {
            console.log(`Drawing ${this.simulation.entities.size} entities`);
            
            // Convert the entity updates to renderable entities
            for (const entity of this.simulation.entities.values()) {
                // Make sure we have required fields
                if (!entity.position || !entity.entity_type) {
                    console.log("Skipping entity without position or type:", entity);
                    continue;
                }
                
                console.log("Processing entity for drawing:", entity.entity_id, 
                            "at", entity.position.x, entity.position.y,
                            "canvas coords:", entity.position.x * scaleX, entity.position.y * scaleY);
                
                // Apply zoom and panning to the coordinates
                const worldX = entity.position.x;
                const worldY = entity.position.y;
                
                // Transform world coordinates to screen coordinates with zoom and pan
                const screenX = (worldX - viewportX) * scaleX;
                const screenY = (worldY - viewportY) * scaleY;
                
                // Create an entity object for rendering
                entities.push({
                    id: entity.entity_id,
                    type: entity.entity_type,
                    x: screenX,
                    y: screenY,
                    action: entity.current_action,
                    drives: entity.drives,
                    // Store original position for reference
                    worldX,
                    worldY
                });
            }
        } else {
            console.log("No entities found in simulation state");
        }
        
        // If no entities found, show some placeholder entities
        if (entities.length === 0) {
            // Apply zoom and panning to the coordinates
            const worldPositions = [
                { type: 'NPC', id: 'npc_1', worldX: 100, worldY: 100 },
                { type: 'NPC', id: 'npc_2', worldX: 200, worldY: 150 },
                { type: 'Object', id: 'food_1', worldX: 150, worldY: 250 },
                { type: 'Object', id: 'shelter_1', worldX: 300, worldY: 200 }
            ];
            
            // Convert world coordinates to screen coordinates
            const placeholders = worldPositions.map(p => ({
                type: p.type,
                id: p.id,
                x: (p.worldX - viewportX) * scaleX,
                y: (p.worldY - viewportY) * scaleY,
                worldX: p.worldX,
                worldY: p.worldY
            }));
            
            entities.push(...placeholders);
        }
        
        // Use the active actions map that we've built during event processing
        const activeActions = this.simulation.activeActions;
        
        console.log(`Drawing ${entities.length} entities`);
        
        // Draw all entities
        entities.forEach(entity => {
            let color;
            let size = 12;
            let shape = 'circle'; // Default shape
            
            console.log(`Drawing entity: ${entity.id}, type: ${entity.type}, at (${entity.x}, ${entity.y})`);
            
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
        const screenX = event.clientX - rect.left;
        const screenY = event.clientY - rect.top;
        
        // Find if any entity was clicked
        if (!this.simulation || this.simulation.entities.size === 0) {
            return;
        }
        
        // Get current viewport settings
        const zoomLevel = this.simulation.zoomLevel;
        const viewportX = this.simulation.viewportX;
        const viewportY = this.simulation.viewportY;
        
        // Use the world size from the SIMULATION_START event if available
        const worldSize = this.simulation.worldSize || WORLD_SIZE;
        
        // Scale factors for canvas to world conversion with zoom
        const scaleX = (this.canvas.width / worldSize) * zoomLevel;
        const scaleY = (this.canvas.height / worldSize) * zoomLevel;
        
        // Convert screen coordinates to world coordinates
        const worldX = (screenX / scaleX) + viewportX;
        const worldY = (screenY / scaleY) + viewportY;
        
        console.log(`Click at screen (${screenX}, ${screenY}), world (${worldX.toFixed(1)}, ${worldY.toFixed(1)})`);
        
        // Find the clicked entity
        let clickedEntity = null;
        let closestDistance = 30 / zoomLevel; // Detection radius adjusts with zoom level
        
        for (const entity of this.simulation.entities.values()) {
            if (!entity.position) continue;
            
            // Use world coordinates for distance calculation
            const entityX = entity.position.x;
            const entityY = entity.position.y;
            
            // Calculate distance in world coordinates
            const distance = Math.sqrt(
                Math.pow(entityX - worldX, 2) + 
                Math.pow(entityY - worldY, 2)
            );
            
            console.log(`Entity ${entity.entity_id} at (${entityX}, ${entityY}), distance: ${distance}`);
            
            // If this is the closest entity within our threshold, select it
            if (distance < closestDistance) {
                closestDistance = distance;
                clickedEntity = entity;
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