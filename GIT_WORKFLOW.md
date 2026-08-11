# RavenMUD Git Workflow (L6 Architecture)

This document outlines the strategy for managing the RavenMUD codebase across public (open-source) and private (production) environments.

## 1. Repository Architecture

We utilize a **Dual-Remote, Two-Branch** strategy to ensure that engine improvements can be shared with the community while proprietary world data and status secrets remain secure.

### Branches
*   **`main` (Public/Community):** Contains the core game engine and the standard open-source world files (matching `raven2`). Proprietary zones and real secrets are **excluded**.
*   **`production` (Private/Full-Game):** Inherits from `main` but includes the **full proprietary game world** and the actual **`secrets.c/h`** data.
*   **`master` (Historical):** A legacy branch preserved on the public `raven2` repository as a 12-year historical record (circa 2012). It is **not** used for active development.

### Remotes
*   **`origin`:** Points to `raven2-private` (GitHub Private). Receives both `main` and `production` branches.
*   **`public`:** Points to `craklyn/raven2` (GitHub Public). Receives **only** the `main` branch.

## 2. Safety Safeguards

1.  **Isolated Secrets:** Real status/chore data is kept in `src/include/general/secrets.h` and `src/lib/general/secrets.c`.
2.  **Explicit Aliases:**
    *   `git push-prod`: Safely pushes to the private repository.
    *   `git push-public`: Pushes the clean `main` branch to open-source.
3.  **Pre-Push Hook:** A gatekeeper script at `.githooks/pre-push` scans every outgoing push to the `public` remote and aborts if it detects any secret files or proprietary branches.

## 3. Deployment Workflow

1.  **Develop on `main`:** All code changes (like the Elemancer) should be done on `main`.
2.  **Sync to Public:** Run `git push-public`.
3.  **Merge to Production:**
    ```bash
    git checkout production
    git merge main
    git push-prod
    ```
4.  **Deploy to EC2:** SSH to the server and `git pull origin production`.

## 4. Proprietary Data Boundary

The following paths are considered **Proprietary** and are only tracked in the `production` branch:
*   Real `secrets.c` and `secrets.h`.
*   All zones in `sys/world` not present in the open-source baseline.
*   Legacy administrative logs and feedback.
