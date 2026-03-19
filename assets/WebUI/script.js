document.addEventListener('DOMContentLoaded', () => {

  // Initialize Custom JS Selectors as Horizontal Sliders
  const nativeSelects = document.querySelectorAll('select.custom-select');
  nativeSelects.forEach(select => {
    select.style.display = 'none';
    
    const wrapper = document.createElement('div');
    wrapper.className = 'custom-select-wrapper ' 
      + (select.classList.contains('wide') ? 'wide ' : '') 
      + (select.classList.contains('top-selector') ? 'top-selector' : '');
      
    select.parentNode.insertBefore(wrapper, select);
    wrapper.appendChild(select);
    
    const trigger = document.createElement('div');
    trigger.className = 'custom-select-trigger';
    
    // Left arrow button
    const leftArrow = document.createElement('div');
    leftArrow.innerHTML = '&#9664;'; // <
    leftArrow.style.color = '#00e5ff';
    leftArrow.style.fontSize = '8px';
    leftArrow.style.opacity = '0.7';
    leftArrow.style.padding = '0 6px';
    leftArrow.style.cursor = 'pointer';
    trigger.appendChild(leftArrow);
    
    // Container to hold text for sliding animation
    const textContainer = document.createElement('div');
    textContainer.style.position = 'relative';
    textContainer.style.overflow = 'hidden';
    textContainer.style.flexGrow = '1'; /* Take up middle space */
    textContainer.style.height = '14px';
    textContainer.style.display = 'flex';
    textContainer.style.alignItems = 'center';
    textContainer.style.justifyContent = 'center';

    const textSpan = document.createElement('span');
    textSpan.textContent = select.options[select.selectedIndex].text;
    textSpan.style.position = 'absolute';
    textSpan.style.width = '100%';
    textSpan.style.textAlign = 'center';
    textContainer.appendChild(textSpan);
    trigger.appendChild(textContainer);
    
    // Right arrow button
    const rightArrow = document.createElement('div');
    rightArrow.innerHTML = '&#9654;'; // >
    rightArrow.style.color = '#00e5ff';
    rightArrow.style.fontSize = '8px';
    rightArrow.style.opacity = '0.7';
    rightArrow.style.padding = '0 6px';
    rightArrow.style.cursor = 'pointer';
    trigger.appendChild(rightArrow);
    
    wrapper.appendChild(trigger);
    
    let isDragging = false;
    let startX = 0;
    let currentIndex = select.selectedIndex;
    
    let juceCombo = null;
    if (select.id && typeof window.Juce !== 'undefined' && window.Juce.getComboBoxState) {
        try {
            juceCombo = window.Juce.getComboBoxState(select.id);
            let idx = juceCombo.getChoiceIndex();
            if (idx >= 0 && idx < select.options.length) {
                currentIndex = idx;
                select.selectedIndex = currentIndex;
                textSpan.textContent = select.options[currentIndex].text;
            }
            juceCombo.valueChangedEvent.addListener(() => {
                let idx = juceCombo.getChoiceIndex();
                if (idx >= 0 && idx < select.options.length && idx !== currentIndex) {
                    currentIndex = idx;
                    select.selectedIndex = currentIndex;
                    textSpan.textContent = select.options[currentIndex].text;
                }
            });
        } catch(e) {}
    }
    
    trigger.addEventListener('mousedown', (e) => {
      isDragging = true;
      startX = e.clientX;
      document.body.style.cursor = 'ew-resize';
      e.preventDefault();
    });
    
    window.addEventListener('mousemove', (e) => {
      if (!isDragging) return;
      const deltaX = e.clientX - startX;
      
      // Every 25px dragged changes the option
      if (Math.abs(deltaX) > 25) {
        let newIndex = currentIndex + (deltaX > 0 ? 1 : -1);
        
        if (newIndex >= 0 && newIndex < select.options.length) {
            const direction = deltaX > 0 ? 1 : -1;
            animateTextChange(newIndex, direction);
            currentIndex = newIndex;
            select.selectedIndex = currentIndex;
            if (juceCombo) juceCombo.setChoiceIndex(currentIndex);
            startX = e.clientX; // reset threshold
        }
      }
    });
    
    window.addEventListener('mouseup', () => {
      if (isDragging) {
        isDragging = false;
        document.body.style.cursor = 'default';
      }
    });

    // Handle clicks precisely on arrows
    leftArrow.addEventListener('click', (e) => {
      if (isDragging) return;
      let newIndex = currentIndex - 1;
      if (newIndex >= 0) {
        animateTextChange(newIndex, -1);
        currentIndex = newIndex;
        select.selectedIndex = currentIndex;
        if (juceCombo) juceCombo.setChoiceIndex(currentIndex);
      }
      e.stopPropagation();
    });

    rightArrow.addEventListener('click', (e) => {
      if (isDragging) return;
      let newIndex = currentIndex + 1;
      if (newIndex < select.options.length) {
        animateTextChange(newIndex, 1);
        currentIndex = newIndex;
        select.selectedIndex = currentIndex;
        if (juceCombo) juceCombo.setChoiceIndex(currentIndex);
      }
      e.stopPropagation();
    });

    function animateTextChange(newIdx, direction) {
        const oldSpan = textContainer.children[0];
        const newSpan = document.createElement('span');
        newSpan.textContent = select.options[newIdx].text;
        newSpan.style.position = 'absolute';
        newSpan.style.width = '100%';
        newSpan.style.textAlign = 'center';
        newSpan.style.transition = 'transform 0.15s ease-out, opacity 0.15s ease-out';
        
        // start new text slightly offset
        newSpan.style.transform = `translateX(${direction * 30}px)`;
        newSpan.style.opacity = '0';
        
        oldSpan.style.transition = 'transform 0.15s ease-out, opacity 0.15s ease-out';
        
        textContainer.appendChild(newSpan);
        
        // reflow
        void newSpan.offsetWidth;
        
        // slide in
        newSpan.style.transform = 'translateX(0)';
        newSpan.style.opacity = '1';
        // slide out
        oldSpan.style.transform = `translateX(${-direction * 30}px)`;
        oldSpan.style.opacity = '0';
        
        setTimeout(() => {
            if (oldSpan.parentNode) {
                oldSpan.remove();
            }
        }, 150);
    }
  });

  // Position circular 0-10 markers around knobs
  const fullMarkerContainers = document.querySelectorAll('.knob-markers.full');
  fullMarkerContainers.forEach(container => {
    const spans = container.querySelectorAll('span');
    const radius = 35; // Tucked tight around the 55px (27.5 r) knob so strings don't overlap siblings
    const startAngle = -135; 
    const endAngle = 135;
    const angleRange = endAngle - startAngle;
    
    spans.forEach((span, i) => {
      const pct = i / (spans.length - 1);
      const angleDeg = startAngle + (pct * angleRange);
      // 0deg is top, map to standard trigonometric circle (0 is right, -90 is top)
      const angleRad = (angleDeg - 90) * (Math.PI / 180);
      
      const x = Math.cos(angleRad) * radius;
      const y = Math.sin(angleRad) * radius;
      
      span.style.transform = `translate(calc(-50% + ${x}px), calc(-50% + ${y}px))`;
    });
  });

  // Initialize Knobs
  const knobs = document.querySelectorAll('.knob.has-ring');
  
  knobs.forEach(knob => {
    // Read data attributes
    const min = parseFloat(knob.dataset.min);
    const max = parseFloat(knob.dataset.max);
    let val = parseFloat(knob.dataset.value);
    
    // Create ring elements
    const ring = document.createElement('div');
    ring.className = 'knob-ring';
    const ringGlow = document.createElement('div');
    ringGlow.className = 'knob-ring-glow';
    
    knob.appendChild(ringGlow);
    knob.appendChild(ring);
    
    const juceIds = {
      'f1-cutoff': 'f1Cutoff', 'f1-reso': 'f1Reso',
      'f2-cutoff': 'f2Cutoff', 'f2-reso': 'f2Reso'
    };
    const backendId = juceIds[knob.id] || knob.id;
    
    let juceState = null;
    if (backendId && typeof window.Juce !== 'undefined' && window.Juce.getSliderState) {
        try {
            juceState = window.Juce.getSliderState(backendId);
            val = min + juceState.getNormalisedValue() * (max - min); // Rescale normalized 0..1 to min..max
            juceState.valueChangedEvent.addListener(() => {
                val = min + juceState.getNormalisedValue() * (max - min);
                if (val < min) val = min;
                if (val > max) val = max;
                updateVisuals();
            });
        } catch(e) {}
    }
    
    // Update visuals based on value
    const updateVisuals = () => {
      // Calculate percentage (0 to 1)
      let pct = (val - min) / (max - min);
      if (pct < 0) pct = 0; if (pct > 1) pct = 1;
      
      // Map to angle (-135deg to +135deg is a 270deg sweep)
      const startAngle = -135;
      const endAngle = 135;
      const angleRange = endAngle - startAngle;
      
      const currentAngle = startAngle + (pct * angleRange);
      
      // The CSS conic-gradient API: 
      // 0deg is top. We want the ring to start at bottom-left (-135deg from center top) and fill clockwise.
      // So the "empty" part is from `currentAngle` back to `-135`.
      // Conic starts at 0. Let's just draw the active part in cyan and the rest transparent.
      // From 225deg (-135deg rotated) to whatever current angle is mapped to 0-360.
      
      // Conic gradient syntax: start angle is 0 by default. 
      // We start the sweep from 225deg (bottom left).
      const sweepAngle = pct * 270;
      
      const grad = `conic-gradient(from 225deg, #00e5ff 0deg, #00e5ff ${sweepAngle}deg, transparent ${sweepAngle}deg)`;
      
      ring.style.background = grad;
      ringGlow.style.background = grad;
      
      // Rotate the knob cap indicator. It starts pointing perfectly up, which is 0.
      // So we rotate by `currentAngle`.
      // Actually we use inline transform on a pseudo element, but pseudo elements can't be styled inline easily.
      // So we just rotate the knob element itself! The ring rotates with it?
      // Ah. If we rotate the knob, the ring rotates. We don't want the START of the ring to move.
      // So we rotate ONLY an indicator element. Wait, earlier CSS added `.knob::before` for the line.
      // We can use a CSS variable `--rot` on the knob, and style `.knob::before { transform: translateX(-50%) rotate(var(--rot)); }`
      knob.style.setProperty('--rot', `${currentAngle}deg`);
      
      // Trigger global graph redraw based on new values
      if (typeof drawGraph === 'function') drawGraph();
    };
    
    updateVisuals();
    
    // Drag interaction
    let isDragging = false;
    let startY = 0;
    let startVal = 0;
    
    knob.addEventListener('mousedown', (e) => {
      isDragging = true;
      startY = e.clientY;
      startVal = val;
      if (juceState) juceState.sliderDragStarted();
      document.body.style.cursor = 'ns-resize';
      e.preventDefault();
    });
    
    window.addEventListener('mousemove', (e) => {
      if (!isDragging) return;
      
      const deltaY = startY - e.clientY;
      const sensitivity = (max - min) * 0.005; 
      
      val = startVal + (deltaY * sensitivity);
      if (val < min) val = min;
      if (val > max) val = max;
      
      if (juceState) {
          let norm = (val - min) / (max - min);
          juceState.setNormalisedValue(norm);
      }
      
      updateVisuals();
    });
    
    window.addEventListener('mouseup', () => {
      if (isDragging) {
         if (juceState) juceState.sliderDragEnded();
         isDragging = false;
         document.body.style.cursor = 'default';
      }
    });
  });

  // Inject CSS for the indicator line variable rotation
  const style = document.createElement('style');
  style.textContent = `
    .knob::before {
      transform: translateX(-50%) rotate(var(--rot, -135deg));
    }
  `;
  document.head.appendChild(style);

  function getKnobPct(id) {
    const knob = document.getElementById(id);
    if (!knob) return 0.5;
    const angle = parseFloat(knob.dataset.angle) || -135;
    // Map -135 to 135 into 0.0 to 1.0
    return (angle + 135) / 270;
  }

  function drawGraph() {
    const canvas = document.getElementById('freq-canvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;
    
    // Clear
    ctx.clearRect(0, 0, width, height);

    // Read knob normalized values (0 to 1)
    const f1Cut = getKnobPct('f1-cutoff');
    const f1Res = getKnobPct('f1-reso');
    const f2Cut = getKnobPct('f2-cutoff');
    const f2Res = getKnobPct('f2-reso');

    // Synth Graphic EQ Math
    ctx.beginPath();
    ctx.lineWidth = 2.5;
    ctx.strokeStyle = '#00e5ff';
    ctx.shadowColor = '#00e5ff';
    ctx.shadowBlur = 10;
    
    // Baseline -20dB is near the bottom
    const floorY = height - 10;

    for (let x = 0; x < width; x++) {
      let y = 70; // 0dB baseline is near the middle/top
      
      // Filter 1 is Low-Pass
      const f1Freq = 20 + f1Cut * (width - 40);
      let f1Atten = 0;
      if (x > f1Freq) f1Atten = Math.pow(x - f1Freq, 1.2) * 0.4; // 12dB slope down
      
      // Filter 1 Resonance bump
      let f1Width = 80 - (f1Res * 50); // Resonance sharpens the bell
      let f1Dist = Math.abs(x - f1Freq);
      if (f1Dist < f1Width && f1Res > 0.01) {
         let bell = Math.cos((f1Dist / f1Width) * (Math.PI / 2));
         f1Atten -= Math.pow(bell, 2) * (f1Res * 60); // Peak height pushes graph UP (minus Y)
      }
      
      // Filter 2 is High-Pass
      const f2Freq = 20 + f2Cut * (width - 40);
      let f2Atten = 0;
      if (x < f2Freq) f2Atten = Math.pow(f2Freq - x, 1.2) * 0.4; // HP Slope down
      
      // Filter 2 Resonance bump
      let f2Width = 80 - (f2Res * 50);
      let f2Dist = Math.abs(x - f2Freq);
      if (f2Dist < f2Width && f2Res > 0.01) {
         let bell = Math.cos((f2Dist / f2Width) * (Math.PI / 2));
         f2Atten -= Math.pow(bell, 2) * (f2Res * 60);
      }
      
      // Combine attenuation and bumps
      y += f1Atten + f2Atten;

      // Restrict drawing bounds
      if (y > floorY) y = floorY;
      if (y < 10) y = 10;

      if (x === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    }
    
    ctx.stroke();
  }

  // Draw Context Graph Initially
  setTimeout(() => { if (typeof drawGraph === 'function') drawGraph(); }, 100);

});
