# Dpaper Publishing Plan

## Current Status
- ✅ Core application complete (C, GTK3, Ayatana AppIndicator)
- ✅ 34 default wallpapers bundled
- ✅ Find/Remove photos functionality
- ✅ Auto-rotate with configurable interval
- ✅ KDE Plasma integration (plasma-apply-wallpaperimage + D-Bus)
- ✅ System tray menu with all features
- ✅ JSON configuration system
- ✅ Makefile-based installation
- ✅ Smart package installer (pkg.sh)

## Publishing Options

### Option 1: GitHub Releases (Recommended - Free)
**Steps:**
1. Create GitHub repository: `github.com/yourusername/dpaper`
2. Tag version: `git tag v1.0.0`
3. Push: `git push origin main --tags`
4. Create release on GitHub UI
5. Attach generated packages

**Assets to upload:**
- `pkg/kde-dp-1.0.tar.gz` (smart installer)
- Source tarball: `dpaper-1.0.0.tar.gz`

**Pros:** Free, easy, community visibility, GitHub Actions for CI/CD
**Cons:** Manual package management for users

---

### Option 2: AUR (Arch User Repository)
**Steps:**
1. Create `dpaper-git` or `dpaper` PKGBUILD
2. Submit to AUR

**Example PKGBUILD:**
```bash
pkgname=dpaper
pkgver=1.0.0
pkgrel=1
arch=('x86_64')
depends=('gtk3' 'libayatana-appindicator')
source=("https://github.com/YOURNAME/dpaper/archive/v${pkgver}.tar.gz")
build() {
  cd "$pkgname-$pkgver/dp"
  make
}
package() {
  cd "$pkgname-$pkgver/dp"
  make install PREFIX=/usr
}
```

**Pros:** One-command install for Arch users (`yay -S dpaper`)
**Cons:** Arch-only

---

### Option 3: PPA (Personal Package Archive) for Ubuntu/Debian
**Steps:**
1. Create account on [launchpad.net](https://launchpad.net)
2. Create PPA: `~yourname/ubuntu/dpaper`
3. Install `devscripts` and `dh-make`
4. Create debian packaging files
5. Upload with `dput`

**Pros:** Easy install for Ubuntu/Debian users (`sudo add-apt-repository ppa:yourname/dpaper && sudo apt update && sudo apt install dpaper`)
**Cons:** Requires Ubuntu/Debian focus, more packaging work

---

### Option 4: Flathub (Flatpak) - Cross-distribution
**Steps:**
1. Fork flathub template repo
2. Add dpaper metadata and dependencies
3. Submit PR to flathub

**Pros:** Works on ANY Linux distribution
**Cons:** Larger download size (~50MB+), sandbox restrictions

---

### Option 5: Snap Store
**Steps:**
1. Create Snapcraft account
2. `snapcraft login`
3. `snapcraft upload pkg/kde-dp-1.0.tar.gz`
4. Release to stable channel

**Pros:** Cross-distribution, auto-updates
**Cons:** Snapd required, KDE/GTK apps have larger overhead

---

## Recommended Publishing Strategy

### Phase 1: Immediate (This Week)
1. **GitHub Release** - Create repo, tag v1.0.0, upload packages
2. **AUR Package** - Submit PKGBUILD for Arch users

### Phase 2: Short-term (1-2 weeks)
3. **PPA** - Create Ubuntu/Debian package
4. **Update docs** - Add installation instructions to README

### Phase 3: Medium-term (1 month)
5. **Flathub** - Submit to Flathub for cross-distribution
6. **Website** - Simple landing page at dpaper.io (optional)

## Quick Start Commands

```bash
# Create GitHub release
git tag v1.0.0
git push origin main --tags

# Create source tarball
git archive --format=tar.gz --prefix=dpaper-1.0.0/ v1.0.0 > ../dpaper-1.0.0.tar.gz

# Verify packages
ls -lh pkg/kde-dp-1.0.tar.gz
ls -lh ../dpaper-1.0.0.tar.gz
```

## Next Steps

1. **Choose platform** - Which option(s) do you want to pursue?
2. **GitHub account** - Do you have a GitHub account for the repo?
3. **Package testing** - Test the smart installer on a clean system
4. **Version numbering** - Confirm v1.0.0 or prefer different version?

Let me know which direction you'd like to go!