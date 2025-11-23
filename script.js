// API Configuration
const API_BASE = 'http://localhost:8080/api';

// Data Structures (for frontend state)
class Book {
    constructor(id, title, author, available = true) {
        this.id = parseInt(id);
        this.title = title;
        this.author = author;
        this.available = available;
    }
}

class Student {
    constructor(bookId, name, bookTitle, dateIssued, dueDate) {
        this.bookId = parseInt(bookId);
        this.name = name;
        this.bookTitle = bookTitle;
        this.dateIssued = dateIssued;
        this.dueDate = dueDate;
    }
}

// State Management
const AppState = {
    books: [],
    students: [],
    history: [],
    admin: { username: 'admin', password: 'admin' },
    currentUser: null
};

// API Functions
async function fetchBooks() {
    try {
        const response = await fetch(`${API_BASE}/books`);
        const data = await response.json();
        AppState.books = data.map(b => new Book(b.id, b.title, b.author, b.available));
        renderBooksTable();
        updateStats();
    } catch (error) {
        console.error('Error fetching books:', error);
        showNotification('Failed to connect to server. Make sure server.exe is running!', 'error');
    }
}

async function fetchStudents() {
    try {
        const response = await fetch(`${API_BASE}/students`);
        const data = await response.json();
        AppState.students = data.map(s => new Student(s.bookId, s.name, s.bookTitle, s.dateIssued, s.dueDate));
        renderStudentsTable();
        updateStats();
    } catch (error) {
        console.error('Error fetching students:', error);
    }
}

async function addBookAPI(id, title, author) {
    try {
        const response = await fetch(`${API_BASE}/books`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: parseInt(id), title, author })
        });
        return response.ok;
    } catch (error) {
        console.error('Error adding book:', error);
        return false;
    }
}

async function issueBookAPI(bookId, studentName, days) {
    try {
        const response = await fetch(`${API_BASE}/issue`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ bookId: parseInt(bookId), studentName, days: parseInt(days) })
        });
        return response.ok;
    } catch (error) {
        console.error('Error issuing book:', error);
        return false;
    }
}

async function returnBookAPI(bookId) {
    try {
        const response = await fetch(`${API_BASE}/return`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ bookId: parseInt(bookId) })
        });
        return response.ok;
    } catch (error) {
        console.error('Error returning book:', error);
        return false;
    }
}

async function deleteBookAPI(id) {
    try {
        const response = await fetch(`${API_BASE}/books/${id}`, {
            method: 'DELETE'
        });
        return response.ok;
    } catch (error) {
        console.error('Error deleting book:', error);
        return false;
    }
}

// DOM Elements
const createAdminScreen = document.getElementById('create-admin-screen');
const loginScreen = document.getElementById('login-screen');
const dashboardScreen = document.getElementById('dashboard-screen');
const createAdminForm = document.getElementById('create-admin-form');
const loginForm = document.getElementById('login-form');
const navLinks = document.querySelectorAll('.nav-links li');
const views = document.querySelectorAll('.view');
const logoutBtn = document.getElementById('logout-btn');

// Initialization
document.addEventListener('DOMContentLoaded', async () => {
    // Show login screen
    loginScreen.classList.add('active');

    // Try to connect to server
    await fetchBooks();
    await fetchStudents();
});

// Auth Logic

// 1. Create Admin (not needed since C backend has hardcoded admin)
createAdminForm.addEventListener('submit', (e) => {
    e.preventDefault();
    showNotification('Admin already exists in C backend (admin/admin)', 'info');
    createAdminScreen.classList.remove('active');
    loginScreen.classList.add('active');
});

// 2. Login
loginForm.addEventListener('submit', (e) => {
    e.preventDefault();
    const user = document.getElementById('username').value;
    const pass = document.getElementById('password').value;

    if (user === AppState.admin.username && pass === AppState.admin.password) {
        AppState.currentUser = user;
        loginScreen.classList.remove('active');
        dashboardScreen.classList.add('active');
        showNotification(`Welcome back, ${user}!`, 'success');

        // Refresh data
        fetchBooks();
        fetchStudents();
    } else {
        showNotification('Invalid credentials! Use admin/admin', 'error');
    }
});

logoutBtn.addEventListener('click', () => {
    AppState.currentUser = null;
    dashboardScreen.classList.remove('active');
    loginScreen.classList.add('active');
    loginForm.reset();
});

// Navigation Logic
navLinks.forEach(link => {
    link.addEventListener('click', () => {
        navLinks.forEach(l => l.classList.remove('active'));
        link.classList.add('active');

        const viewId = `view-${link.dataset.view}`;
        views.forEach(view => {
            view.classList.remove('active');
            if (view.id === viewId) view.classList.add('active');
        });
    });
});

// Password Reset
document.getElementById('reset-password-form').addEventListener('submit', async (e) => {
    e.preventDefault();

    const oldPassword = document.getElementById('old-password').value;
    const newUsername = document.getElementById('new-username').value;
    const newPassword = document.getElementById('new-password').value;
    const confirmPassword = document.getElementById('confirm-password').value;

    if (newPassword !== confirmPassword) {
        showNotification('Passwords do not match!', 'error');
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/reset-password`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                oldPassword,
                newUsername,
                newPassword
            })
        });

        const result = await response.json();

        if (response.ok && result.success) {
            showNotification('Credentials updated! Please log in again.', 'success');
            AppState.admin = { username: newUsername, password: newPassword };

            // Log out and redirect to login
            setTimeout(() => {
                AppState.currentUser = null;
                dashboardScreen.classList.remove('active');
                loginScreen.classList.add('active');
                document.getElementById('reset-password-form').reset();
            }, 1500);
        } else {
            showNotification(result.error || 'Failed to update credentials', 'error');
        }
    } catch (error) {
        console.error('Error resetting password:', error);
        showNotification('Failed to connect to server', 'error');
    }
});


// Core Logic

// 1. Add Book
document.getElementById('add-book-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const id = document.getElementById('new-book-id').value;
    const title = document.getElementById('new-book-title').value;
    const author = document.getElementById('new-book-author').value;

    // Check ID uniqueness
    if (AppState.books.some(b => b.id == id)) {
        showNotification('Book ID already exists!', 'error');
        return;
    }

    const success = await addBookAPI(id, title, author);
    if (success) {
        showNotification('Book added successfully!', 'success');
        e.target.reset();
        await fetchBooks();
    } else {
        showNotification('Failed to add book', 'error');
    }
});

// 2. Issue Book
document.getElementById('issue-book-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const id = parseInt(document.getElementById('issue-book-id').value);
    const studentName = document.getElementById('issue-student-name').value;
    const days = document.getElementById('issue-days').value || 14;

    const book = AppState.books.find(b => b.id === id);

    if (!book) {
        showNotification('Book not found!', 'error');
        return;
    }
    if (!book.available) {
        showNotification('Book already issued!', 'warning');
        return;
    }

    const success = await issueBookAPI(id, studentName, days);
    if (success) {
        showNotification(`Book issued to ${studentName}`, 'success');
        e.target.reset();
        document.getElementById('issue-days').value = 14;
        await fetchBooks();
        await fetchStudents();
    } else {
        showNotification('Failed to issue book', 'error');
    }
});

// 3. Return Book
document.getElementById('return-book-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const id = parseInt(document.getElementById('return-book-id').value);

    const success = await returnBookAPI(id);
    if (success) {
        showNotification('Book returned successfully!', 'success');
        e.target.reset();
        await fetchBooks();
        await fetchStudents();
    } else {
        showNotification('No record found for this book ID.', 'error');
    }
});

// 4. Delete Book
window.deleteBook = async (id) => {
    if (!confirm('Are you sure you want to delete this book?')) return;

    const success = await deleteBookAPI(id);
    if (success) {
        showNotification('Book deleted.', 'success');
        await fetchBooks();
    } else {
        showNotification('Failed to delete book', 'error');
    }
};

// 5. Search
document.getElementById('global-search').addEventListener('input', (e) => {
    const term = e.target.value.toLowerCase();
    renderBooksTable(term);
});

// UI Helpers
function renderBooksTable(filter = '') {
    const tbody = document.getElementById('books-table-body');
    tbody.innerHTML = '';

    const filteredBooks = AppState.books.filter(b =>
        b.title.toLowerCase().includes(filter) ||
        b.author.toLowerCase().includes(filter) ||
        b.id.toString().includes(filter)
    );

    filteredBooks.forEach(book => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>#${book.id}</td>
            <td>${book.title}</td>
            <td>${book.author}</td>
            <td><span class="status-badge ${book.available ? 'status-available' : 'status-issued'}">
                ${book.available ? 'Available' : 'Issued'}
            </span></td>
            <td>
                <button class="action-btn delete" onclick="deleteBook(${book.id})">
                    <i class="fa-solid fa-trash"></i>
                </button>
            </td>
        `;
        tbody.appendChild(tr);
    });
}

function renderStudentsTable() {
    const tbody = document.getElementById('students-table-body');
    tbody.innerHTML = '';

    const now = new Date();

    AppState.students.forEach(student => {
        const dueDate = new Date(student.dueDate);
        const isOverdue = now > dueDate;

        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>#${student.bookId}</td>
            <td>${student.name}</td>
            <td>${student.bookTitle}</td>
            <td>${student.dateIssued}</td>
            <td>${student.dueDate}</td>
            <td>
                <span class="status-badge ${isOverdue ? 'status-issued' : 'status-available'}" 
                      style="background: ${isOverdue ? 'rgba(239, 68, 68, 0.2)' : ''}; color: ${isOverdue ? '#ef4444' : ''}">
                    ${isOverdue ? 'Overdue' : 'Active'}
                </span>
            </td>
        `;
        tbody.appendChild(tr);
    });
}

function renderHistoryTable() {
    const tbody = document.getElementById('history-table-body');
    if (!tbody) return;
    tbody.innerHTML = '';

    AppState.history.forEach(entry => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${new Date(entry.date).toLocaleString()}</td>
            <td><span class="status-badge" style="background: rgba(99, 102, 241, 0.1); color: var(--primary)">${entry.action}</span></td>
            <td>${entry.bookTitle}</td>
            <td>${entry.studentName}</td>
        `;
        tbody.appendChild(tr);
    });
}

function updateStats() {
    document.getElementById('total-books').textContent = AppState.books.length;
    document.getElementById('available-books').textContent = AppState.books.filter(b => b.available).length;

    const now = new Date();
    const overdueCount = AppState.students.filter(s => new Date(s.dueDate) < now).length;
    document.getElementById('overdue-books').textContent = overdueCount;
    document.getElementById('issued-books').textContent = AppState.books.filter(b => !b.available).length;
}

function showNotification(msg, type = 'info') {
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    toast.style.position = 'fixed';
    toast.style.bottom = '20px';
    toast.style.right = '20px';
    toast.style.padding = '12px 24px';
    toast.style.background = type === 'success' ? '#10b981' : type === 'error' ? '#ef4444' : type === 'warning' ? '#f59e0b' : '#3b82f6';
    toast.style.color = 'white';
    toast.style.borderRadius = '8px';
    toast.style.zIndex = '1000';
    toast.style.boxShadow = '0 4px 6px rgba(0,0,0,0.1)';
    toast.textContent = msg;

    document.body.appendChild(toast);

    setTimeout(() => {
        toast.remove();
    }, 3000);
}
