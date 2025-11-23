# Library Management System

A modern, full-stack Library Management System with a C backend and beautiful web frontend.

> **⚠️ Important Note About Deployment:**  
> This application requires a **C backend server** running locally. The GitHub Pages deployment shows the frontend only.  
> To use the full application with all features, you must **run it locally** by following the installation instructions below.

## Features

- 📚 **Book Management**: Add, view, and delete books
- 👥 **Borrower Tracking**: Issue and return books with due dates
- 📊 **Dashboard Statistics**: Real-time stats on books, availability, and overdue items
- 🔐 **Admin Authentication**: Secure login with customizable credentials
- 🎨 **Premium UI**: Modern glassmorphism design with smooth animations
- 🔍 **Search Functionality**: Quick search across books, authors, and IDs

## Technology Stack

### Backend
- **Language**: C
- **Server**: Custom HTTP server using Winsock2
- **Port**: 8080
- **API**: RESTful JSON API

### Frontend
- **HTML5** with semantic markup
- **CSS3** with modern design (glassmorphism, gradients, animations)
- **Vanilla JavaScript** for dynamic interactions
- **Font Awesome** icons
- **Google Fonts** (Outfit)

## Getting Started

### Prerequisites
- Windows OS
- GCC compiler (MinGW recommended)
- Modern web browser (Chrome, Firefox, Edge)

### Installation & Running

1. **Start the Backend Server**
   ```bash
   # Double-click the batch file:
   start_server.bat
   
   # OR run manually:
   library_server.exe
   ```
   
   The server will start on `http://localhost:8080`
   
   Default admin credentials:
   - Username: `admin`
   - Password: `admin`

2. **Open the Frontend**
   - Simply open `index.html` in your web browser
   - Or use a local server like Live Server in VS Code

3. **Login**
   - Use the default credentials: `admin` / `admin`
   - You can change these in the Settings page

## API Endpoints

### Authentication
- `POST /api/login` - Authenticate admin user

### Books
- `GET /api/books` - Get all books
- `POST /api/books` - Add a new book
- `DELETE /api/books/:id` - Delete a book

### Borrowers
- `GET /api/students` - Get all borrowers
- `POST /api/issue` - Issue a book to a student
- `POST /api/return` - Return a book

### Settings
- `POST /api/reset-password` - Update admin credentials

## Usage Guide

### Adding a Book
1. Navigate to "Add Book" in the sidebar
2. Enter Book ID (must be unique)
3. Enter Title and Author
4. Click "Add Book"

### Issuing a Book
1. Go to "Issue / Return" section
2. Enter the Book ID
3. Enter Student Name
4. Set duration (default: 14 days)
5. Click "Issue Book"

### Returning a Book
1. Go to "Issue / Return" section
2. Enter the Book ID in the Return section
3. Click "Return Book"

### Viewing Statistics
- The Books page shows real-time statistics:
  - Total Books
  - Available Books
  - Overdue Books
  - Issued Books

### Changing Admin Credentials
1. Go to Settings
2. Enter current password
3. Enter new username and password
4. Confirm new password
5. Click "Reset Credentials"
6. You'll be logged out and need to login with new credentials

## Troubleshooting

### "Failed to connect to server" error
- Make sure `library_server.exe` is running
- Check if port 8080 is not being used by another application
- Try restarting the server

### "Failed to add book" error
- Ensure the Book ID is unique (not already in use)
- Make sure all fields are filled
- Check that the server is running

### Login not working
- Default credentials are `admin` / `admin`
- If you changed credentials, use the new ones
- If forgotten, restart the server (credentials reset to default)

## Recompiling the Server

If you make changes to `server.c`:

```bash
gcc server.c -o library_server.exe -lws2_32
```

## Project Structure

```
Library management system/
├── index.html          # Main HTML file
├── style.css           # Styling and animations
├── script.js           # Frontend logic and API calls
├── server.c            # C backend server source
├── library_server.exe  # Compiled server executable
├── start_server.bat    # Quick start script
└── README.md           # This file
```

## Features in Detail

### Dashboard
- Real-time statistics cards
- Searchable book table
- Status badges (Available/Issued)
- Quick delete functionality

### Book Management
- Unique ID validation
- Automatic availability tracking
- Author and title search

### Borrower System
- Automatic date calculation
- Overdue detection
- Student name tracking
- Book title association

### Security
- Password-protected admin panel
- Credential reset functionality
- Session management

## Known Limitations

- Single admin user (no multi-user support)
- Data is stored in memory (resets on server restart)
- No persistent storage (no database)
- Simple JSON parsing (may not handle all edge cases)

## Future Enhancements

- [ ] Database integration (SQLite)
- [ ] Multiple user roles
- [ ] Email notifications for overdue books
- [ ] Book cover images
- [ ] Export reports (PDF/CSV)
- [ ] Fine calculation
- [ ] Book reservation system

## License

This project is open source and available for educational purposes.

## Support

For issues or questions, please check the troubleshooting section above.

---

**Made with ❤️ using C and Modern Web Technologies**
