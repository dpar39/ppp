import { Injectable } from '@angular/core';
import { LocalStorageService } from './local-storage.service';

const DARK_MODE = 'darkMode';

@Injectable({ providedIn: 'root' })
export class SettingsService {
  constructor(private localStorage: LocalStorageService) {}

  getDarkMode() {
    let darkMode = this.localStorage.getItem(DARK_MODE);
    if (darkMode != null) {
      return darkMode;
    }

    // Let's  figure out what's the system preference then
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)');
    return prefersDark.matches;
  }

  setDarkMode(darkMode: boolean) {
    if (darkMode === this.localStorage.getItem(DARK_MODE)) {
      // Nothing to do
      return;
    }
    this.localStorage.setItem(DARK_MODE, darkMode);
  }
}
