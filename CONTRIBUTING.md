# Contributing to 3D HUD Rendering Engine

Thank you for your interest in contributing to the 3D HUD Rendering Engine! This document provides guidelines and instructions for contributing to the project.

## 📋 Code of Conduct

Please be respectful and considerate of others when contributing to this project. We aim to foster an inclusive and welcoming community.

## 🎯 How Can I Contribute?

### Reporting Bugs
If you find a bug, please open an issue with the following information:
1. **Clear description** of the bug
2. **Steps to reproduce** the issue
3. **Expected behavior** vs actual behavior
4. **Environment details** (OS, compiler version, graphics hardware)
5. **Screenshots or logs** if applicable

### Suggesting Features
We welcome feature suggestions! Please include:
1. **Use case** for the feature
2. **Proposed implementation** (if you have ideas)
3. **Alternatives** you've considered

### Code Contributions
1. **Small fixes**: Typos, documentation improvements, minor bug fixes
2. **Feature implementations**: New rendering techniques, platform support, optimizations
3. **Performance improvements**: Profiling, optimization, memory management
4. **Testing**: Unit tests, integration tests, performance benchmarks

## 🔧 Development Workflow

### Prerequisites
- C++17 compatible compiler
- CMake 3.16+
- Conan 2.x
- Git

### Getting Started
1. **Fork** the repository on GitHub
2. **Clone** your fork locally
   ```bash
   git clone https://github.com/YOUR_USERNAME/3d_hud.git
   cd 3d_hud
   ```
3. **Create a branch** for your changes
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/issue-you-are-fixing
   ```
4. **Install dependencies**
   ```bash
   conan install . --build=missing
   ```
5. **Configure the build**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON
   ```
6. **Build the project**
   ```bash
   cmake --build .
   ```

### Making Changes
1. **Write your code** following the project's coding standards
2. **Add tests** for new functionality
3. **Update documentation** as needed
4. **Ensure all tests pass**
   ```bash
   ctest --output-on-failure
   ```

### Coding Standards
- **C++17**: Use modern C++ features appropriately
- **Naming conventions**:
  - Classes: `CamelCase`
  - Functions: `camelCase`
  - Variables: `snake_case`
  - Constants: `UPPER_SNAKE_CASE`
- **Comments**: Use Doxygen-style comments for public APIs
- **Formatting**: Use the project's .clang-format configuration
- **Error handling**: Use the `Result<T>` type for operations that can fail

### Commit Guidelines
1. **Write descriptive commit messages**
2. **Reference issue numbers** when applicable
3. **Keep commits focused** (one logical change per commit)
4. **Use conventional commit format**:
   ```
   type(scope): description

   [optional body]

   [optional footer]
   ```

   **Types**:
   - `feat`: New feature
   - `fix`: Bug fix
   - `docs`: Documentation changes
   - `style`: Code style changes (formatting, etc.)
   - `refactor`: Code refactoring
   - `test`: Adding or updating tests
   - `chore`: Maintenance tasks

### Testing
- **Unit tests**: Test individual components in isolation
- **Integration tests**: Test interactions between components
- **Performance tests**: Benchmark critical code paths
- **Graphics tests**: Validate rendering output

### Submitting a Pull Request
1. **Push your changes** to your fork
   ```bash
   git push origin feature/your-feature-name
   ```
2. **Open a Pull Request** on GitHub
3. **Fill out the PR template** with details about your changes
4. **Link related issues** in the PR description
5. **Request reviews** from maintainers

## 📁 Project Structure

### Important Directories
- `inc/`: Header files organized by module
- `src/`: Source files matching the header structure
- `examples/`: Example usage code
- `tests/`: Test files
- `docs/`: Documentation

### Adding New Features
1. **Design discussion**: Open an issue to discuss the feature design
2. **Implementation plan**: Outline the changes needed
3. **Code review**: Submit changes in small, reviewable chunks
4. **Documentation**: Update relevant documentation
5. **Testing**: Add tests for the new feature

## 🧪 Testing Guidelines

### Running Tests
```bash
# Run all tests
ctest --output-on-failure

# Run specific test
ctest -R "test_name" --output-on-failure

# Run tests with verbose output
ctest -V
```

### Writing Tests
- **Test files** should be in the `tests/` directory
- **Test names** should be descriptive
- **Use fixtures** for setup/teardown
- **Mock dependencies** when testing in isolation

## 📝 Documentation

### Types of Documentation
1. **API Documentation**: Doxygen comments in header files
2. **Usage Examples**: Examples in the `examples/` directory
3. **Design Documents**: Architecture and design decisions in `docs/`
4. **README**: Project overview and quick start guide

### Writing Documentation
- **Keep documentation up to date** with code changes
- **Use clear, concise language**
- **Include code examples** when helpful
- **Cross-reference** related documentation

## 🔍 Code Review Process

1. **Automated checks** (CI) must pass
2. **At least one maintainer** must approve
3. **All comments** must be addressed
4. **Squash and merge** for clean commit history

### Review Checklist
- [ ] Code follows project standards
- [ ] Tests are included and pass
- [ ] Documentation is updated
- [ ] Performance implications considered
- [ ] Backward compatibility maintained

## 🐛 Issue Triage

### Issue Labels
- `bug`: Something isn't working
- `enhancement`: New feature or improvement
- `documentation`: Improvements to documentation
- `question`: Further information is requested
- `wontfix`: This will not be worked on
- `duplicate`: This issue already exists

## 🚀 Release Process

### Versioning
We use [Semantic Versioning](https://semver.org/):
- **Major**: Breaking changes
- **Minor**: New features (backward compatible)
- **Patch**: Bug fixes (backward compatible)

### Release Checklist
- [ ] All tests pass
- [ ] Documentation updated
- [ ] Release notes prepared
- [ ] Version number updated
- [ ] Binary compatibility verified

## ❓ Questions?

If you have questions about contributing:
1. Check the existing documentation
2. Search existing issues and discussions
3. Open a new issue with the `question` label

---

Thank you for contributing to the 3D HUD Rendering Engine! Your help makes this project better for everyone.