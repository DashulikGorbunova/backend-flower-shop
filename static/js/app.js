// Добавляем анимацию появления элементов
function animateOnScroll() {
    const elements = document.querySelectorAll('.product-card, .builder-sidebar, .builder-preview');
    
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.style.animation = 'fadeInUp 0.8s ease-out';
                observer.unobserve(entry.target);
            }
        });
    }, { threshold: 0.1 });
    
    elements.forEach(el => observer.observe(el));
}

// Инициализация анимаций при загрузке
document.addEventListener('DOMContentLoaded', function() {
    animateOnScroll();
});

async function loadProducts(category = '') {
    try {
        const response = await fetch('/api/products');
        const data = await response.json();
        
        const container = document.getElementById('products');
        if (!container) {
            console.error('Элемент products не найден!');
            return;
        }
        
        container.innerHTML = '';
        
        console.log('Загружаем продукты, категория:', category);
        
        let productsToShow = data.products;
        
        // Фильтрация по категориям
        if (category === 'flowers') {
            productsToShow = data.products.filter(p => p.id >= 1 && p.id <= 12);
        } else if (category === 'bouquets') {
            productsToShow = data.products.filter(p => p.id >= 13 && p.id <= 24);
        }
        
        console.log(`Показываем ${productsToShow.length} товаров для категории: ${category}`);
        
        if (productsToShow.length === 0) {
            container.innerHTML = `
                <div style="text-align: center; color: var(--gray); padding: 3rem;">
                    <div style="font-size: 4rem; margin-bottom: 1rem;">🌷</div>
                    <h3>Товары не найдены</h3>
                    <p>В категории "${category}" пока нет товаров</p>
                </div>
            `;
            return;
        }
        
        productsToShow.forEach((product, index) => {
            container.innerHTML += `
                <div class="product-card" style="animation-delay: ${index * 0.1}s">
                    <div class="product-image">
                        ${product.image && product.image.startsWith('/static/') ? 
                            `<img src="${product.image}" alt="${product.name}" loading="lazy" onerror="this.style.display='none'">` : 
                            `<div class="emoji-image">${product.image}</div>`}
                    </div>
                    <h3>${product.name}</h3>
                    <div class="product-price">${product.price} руб.</div>
                    <div class="product-description">${product.description}</div>
                    <button class="add-to-cart" onclick="addToCartWithAnimation(${product.id}, this)">
                        В корзину
                    </button>
                </div>
            `;
        });
        
        animateOnScroll();
        
    } catch (error) {
        console.error('Error:', error);
        const container = document.getElementById('products');
        if (container) {
            container.innerHTML = `
                <div style="text-align: center; color: #dc3545; padding: 2rem;">
                    <h3>Ошибка загрузки</h3>
                    <p>Попробуйте обновить страницу</p>
                </div>
            `;
        }
    }
}

async function loadBuilderFlowers() {
    try {
        const response = await fetch('/api/products');
        const data = await response.json();
        
        const container = document.getElementById('flowers-list');
        container.innerHTML = '';
        
        // Берем цветы для конструктора по ID 101-112
        const builderFlowers = data.products.filter(product => 
            product.id >= 101 && product.id <= 112
        );
        
        builderFlowers.forEach((product, index) => {
            container.innerHTML += `
                <div class="builder-flower-item" style="animation-delay: ${index * 0.1}s">
                    <div class="flower-info">
                        <div class="product-image">
                            ${product.image && product.image.startsWith('/static/') ? 
                                `<img src="${product.image}" alt="${product.name}" onerror="this.style.display='none'">` : 
                                `<div class="emoji-image">${product.image}</div>`}
                        </div>
                        <div class="flower-details">
                            <strong class="flower-name">${product.name}</strong>
                            <div class="flower-description">${product.description}</div>
                            <div class="flower-price">${product.price} руб./шт</div>
                        </div>
                    </div>
                    <div class="flower-controls">
                        <button class="quantity-btn" onclick="decreaseFlower(${product.id})">-</button>
                        <span class="quantity-display" id="quantity-${product.id}">0</span>
                        <button class="quantity-btn" onclick="increaseFlower(${product.id}, '${product.name}', ${product.price}, '${product.image}')">+</button>
                    </div>
                </div>
            `;
        });
        
        // Скрываем сообщение о загрузке
        const loadingMessage = document.getElementById('loading-message');
        if (loadingMessage) {
            loadingMessage.style.display = 'none';
        }
        
    } catch (error) {
        console.error('Error:', error);
        const container = document.getElementById('flowers-list');
        container.innerHTML = `
            <div style="text-align: center; color: #dc3545; padding: 2rem;">
                <h3>Ошибка загрузки</h3>
                <p>Попробуйте обновить страницу</p>
            </div>
        `;
    }
}

let bouquetFlowers = {};
let bouquetTotal = 0;

function increaseFlower(id, name, price, image) {
    if (!bouquetFlowers[id]) {
        bouquetFlowers[id] = {
            id: id,
            name: name,
            price: price,
            image: image,
            quantity: 0
        };
    }
    
    bouquetFlowers[id].quantity++;
    bouquetTotal += price;
    
    updateFlowerDisplay(id);
    updateBouquetPreview();
    showNotification(`+1 ${name}`, 'success');
}

function decreaseFlower(id) {
    if (bouquetFlowers[id] && bouquetFlowers[id].quantity > 0) {
        bouquetFlowers[id].quantity--;
        bouquetTotal -= bouquetFlowers[id].price;
        
        if (bouquetFlowers[id].quantity === 0) {
            delete bouquetFlowers[id];
        }
        
        updateFlowerDisplay(id);
        updateBouquetPreview();
        showNotification(`-1 ${bouquetFlowers[id]?.name || 'цветок'}`, 'info');
    }
}

function updateFlowerDisplay(id) {
    const display = document.getElementById(`quantity-${id}`);
    if (display) {
        display.textContent = bouquetFlowers[id]?.quantity || 0;
    }
}

function updateBouquetPreview() {
    const container = document.getElementById('current-bouquet');
    const priceElement = document.getElementById('bouquet-price');
    
    container.innerHTML = '';
    
    let hasFlowers = false;
    
    Object.values(bouquetFlowers).forEach((flower, index) => {
        if (flower.quantity > 0) {
            hasFlowers = true;
            container.innerHTML += `
                <div class="bouquet-flower-item" style="animation-delay: ${index * 0.1}s">
                    <div class="flower-preview-info">
                        <div class="product-image">
                            ${flower.image && flower.image.startsWith('/static/') ? 
                                `<img src="${flower.image}" alt="${flower.name}">` : 
                                `<div class="emoji-image">${flower.image}</div>`}
                        </div>
                        <div class="flower-preview-details">
                            <strong>${flower.name}</strong>
                            <div class="flower-preview-stats">
                                ${flower.price} руб. × ${flower.quantity} = ${flower.price * flower.quantity} руб.
                            </div>
                        </div>
                    </div>
                    <div class="flower-preview-controls">
                        <button class="quantity-btn" onclick="decreaseFlower(${flower.id})">-</button>
                        <span class="quantity-display">${flower.quantity}</span>
                        <button class="quantity-btn" onclick="increaseFlower(${flower.id}, '${flower.name}', ${flower.price}, '${flower.image}')">+</button>
                        <button class="quantity-btn remove-btn" onclick="removeFlowerFromBouquet(${flower.id})">×</button>
                    </div>
                </div>
            `;
        }
    });
    
    if (!hasFlowers) {
        container.innerHTML = `
            <div class="empty-bouquet">
                <div style="font-size: 3rem; margin-bottom: 1rem;">💐</div>
                <h3>Букет пуст</h3>
                <p>Добавьте цветы из списка слева</p>
            </div>
        `;
    }
    
    priceElement.textContent = bouquetTotal;
    priceElement.style.animation = 'pulse 0.5s ease-in-out';
    setTimeout(() => {
        priceElement.style.animation = '';
    }, 500);
}

function removeFlowerFromBouquet(id) {
    if (bouquetFlowers[id]) {
        const flowerName = bouquetFlowers[id].name;
        bouquetTotal -= bouquetFlowers[id].price * bouquetFlowers[id].quantity;
        delete bouquetFlowers[id];
        
        updateFlowerDisplay(id);
        updateBouquetPreview();
        showNotification(`Удалены все ${flowerName}`, 'warning');
    }
}

async function addBouquetToCart() {
    const flowersInBouquet = Object.values(bouquetFlowers).filter(flower => flower.quantity > 0);
    
    if (flowersInBouquet.length === 0) {
        showNotification('Добавьте цветы в букет!', 'error');
        return;
    }
    
    const button = event.target;
    const originalText = button.textContent;
    button.textContent = 'Добавляем...';
    button.disabled = true;
    
    try {
        const flowerNames = flowersInBouquet.map(flower => 
            `${flower.name} (${flower.quantity} шт)`
        ).join(', ');
        
        const bouquetData = {
            name: `Собранный букет из ${flowersInBouquet.length} видов цветов`,
            price: Number(bouquetTotal.toFixed(2)),
            description: `Состав: ${flowerNames}`
        };
        
        // Отправляем на сервер
        const response = await fetch('/api/cart/custom', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(bouquetData)
        });
        
        if (response.ok) {
            showNotification('🎉 Букет добавлен в корзину!', 'success');
            
            // Сбрасываем конструктор
            bouquetFlowers = {};
            bouquetTotal = 0;
            updateBouquetPreview();
            
            // Сбрасываем счетчики
            document.querySelectorAll('.quantity-display').forEach(display => {
                display.textContent = '0';
            });
            
            // Обновляем корзину
            await loadCart();
        } else {
            throw new Error('Server error');
        }
        
    } catch (error) {
        console.error('Error:', error);
        showNotification('Ошибка при добавлении букета', 'error');
    } finally {
        button.textContent = originalText;
        button.disabled = false;
    }
}

async function addToCartWithAnimation(productId, button) {
    // Анимация кнопки
    button.style.transform = 'scale(0.95)';
    const originalText = button.textContent;
    button.textContent = '✓ Добавлено';
    button.disabled = true;
    
    try {
        const response = await fetch(`/api/cart/add/${productId}`);
        const data = await response.json();
        
        if (data.status === 'success') {
            showNotification('Товар добавлен в корзину! 🛒', 'success');
        }
    } catch (error) {
        console.error('Error:', error);
        showNotification('Ошибка при добавлении', 'error');
    } finally {
        setTimeout(() => {
            button.textContent = originalText;
            button.disabled = false;
            button.style.transform = 'scale(1)';
        }, 1000);
    }
}

async function addToCart(productId) {
    try {
        const response = await fetch(`/api/cart/add/${productId}`);
        await response.json();
    } catch (error) {
        console.error('Error:', error);
    }
}

async function loadCart() {
    try {
        const response = await fetch('/api/cart');
        const data = await response.json();
        
        const container = document.getElementById('cart-items');
        const totalElement = document.getElementById('total-price');
        
        container.innerHTML = '';
        
        if (data.cart.length === 0) {
            container.innerHTML = `
                <div style="text-align: center; color: var(--gray); padding: 3rem;">
                    <div style="font-size: 4rem; margin-bottom: 1rem;">🛒</div>
                    <h3 style="color: var(--dark); margin-bottom: 1rem;">Корзина пуста</h3>
                    <p>Добавьте товары из каталога</p>
                </div>
            `;
        } else {
            // Группируем товары по имени для отображения количества
            const groupedCart = {};
            data.cart.forEach(item => {
                // Для кастомных букетов (ID >= 1000) не группируем
                if (item.id >= 1000) {
                    if (!groupedCart[item.id]) {
                        groupedCart[item.id] = { ...item, quantity: 1 };
                    }
                } else {
                    // Для обычных товаров группируем по имени
                    if (!groupedCart[item.name]) {
                        groupedCart[item.name] = { ...item, quantity: 0 };
                    }
                    groupedCart[item.name].quantity++;
                }
            });
            
            Object.values(groupedCart).forEach((item, index) => {
                const totalPrice = item.price * item.quantity;
                const displayName = item.id >= 1000 ? item.name : `${item.name}${item.quantity > 1 ? ` (${item.quantity} шт)` : ''}`;
                
                container.innerHTML += `
                    <div class="cart-item" style="animation-delay: ${index * 0.1}s">
                        <div class="cart-item-image">
                            ${item.image && item.image.startsWith('/static/') ? 
                                `<img src="${item.image}" alt="${item.name}">` : 
                                `<div class="emoji-image">${item.image}</div>`}
                        </div>
                        <div class="cart-item-info">
                            <div class="cart-item-name">${displayName}</div>
                            ${item.description ? `<div class="cart-item-description">${item.description}</div>` : ''}
                            <div class="cart-item-price">${totalPrice} руб.</div>
                        </div>
                    </div>
                `;
            });
        }
        
        totalElement.textContent = data.total;
        animateOnScroll();
    } catch (error) {
        console.error('Error:', error);
    }
}

async function clearCart() {
    if (confirm('Очистить корзину?')) {
        try {
            await fetch('/api/cart/clear');
            await loadCart();
            showNotification('Корзина очищена 🗑️', 'info');
        } catch (error) {
            console.error('Error:', error);
        }
    }
}

function checkout() {
    const cartItems = document.querySelectorAll('.cart-item');
    if (cartItems.length === 0) {
        showNotification('Корзина пуста!', 'error');
        return;
    }
    
    // Красивое модальное окно
    showNotification('🎉 Заказ оформлен! Скоро с вами свяжутся!', 'success');
    
    // Автоочистка через 2 секунды
    setTimeout(() => {
        clearCart();
    }, 2000);
}

function clearBouquetBuilder() {
    const flowersCount = Object.values(bouquetFlowers).filter(flower => flower.quantity > 0).length;
    
    if (flowersCount > 0) {
        if (confirm('Очистить конструктор букета?')) {
            bouquetFlowers = {};
            bouquetTotal = 0;
            updateBouquetPreview();
            
            // Сбрасываем все счетчики
            document.querySelectorAll('.quantity-display').forEach(display => {
                display.textContent = '0';
            });
            
            showNotification('Конструктор очищен 🧹', 'info');
        }
    }
}

// Улучшенные уведомления
function showNotification(message, type = 'info') {
    // Удаляем старые уведомления
    document.querySelectorAll('.notification').forEach(notification => {
        notification.remove();
    });
    
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    
    notification.innerHTML = message;
    
    document.body.appendChild(notification);
    
    // Анимация появления
    setTimeout(() => {
        notification.style.transform = 'translateX(0)';
        notification.style.opacity = '1';
    }, 100);
    
    // Автоудаление
    setTimeout(() => {
        notification.style.transform = 'translateX(400px)';
        notification.style.opacity = '0';
        setTimeout(() => {
            if (notification.parentNode) {
                document.body.removeChild(notification);
            }
        }, 400);
    }, 3000);
}

// Добавляем CSS для анимаций и эмодзи
const style = document.createElement('style');
style.textContent = `
    @keyframes pulse {
        0% { transform: scale(1); }
        50% { transform: scale(1.05); }
        100% { transform: scale(1); }
    }
    
    @keyframes fadeInUp {
        from {
            opacity: 0;
            transform: translateY(30px);
        }
        to {
            opacity: 1;
            transform: translateY(0);
        }
    }
    
    @keyframes slideInRight {
        from {
            opacity: 0;
            transform: translateX(400px);
        }
        to {
            opacity: 1;
            transform: translateX(0);
        }
    }
    
    .emoji-image {
        font-size: 2rem;
        text-align: center;
        padding: 1rem;
        display: flex;
        align-items: center;
        justify-content: center;
        height: 100%;
    }
    
    .product-image .emoji-image {
        font-size: 3rem;
    }
    
    .cart-item-image .emoji-image {
        font-size: 2.5rem;
    }
`;
document.head.appendChild(style);