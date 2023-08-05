import { EventEmitter, Injectable } from '@angular/core';
import { StorageService } from './storage.service';
import { UnitType } from '../model/datatypes';

const DARK_MODE = 'darkMode';
const PRINT_SETTINGS = 'printSettings';

export class PrintSettings {
  paperColor: string;
  gutterValue: number;
  gutterUnits: UnitType;
  centerAlign: boolean;
}

@Injectable({ providedIn: 'root' })
export class SettingsService {
  public printSettingsChanged: EventEmitter<PrintSettings> = new EventEmitter();
  private _printSettings: PrintSettings;
  private _darkMode?: boolean;

  constructor(private storage: StorageService) {
    this.populateSettings();
  }

  populateSettings() {
    this._printSettings = this.storage.getItem(PRINT_SETTINGS);
    if (!this._printSettings) {
      this._printSettings = {
        paperColor: 'lightgray',
        gutterValue: 0.5,
        gutterUnits: UnitType.mm,
        centerAlign: true,
      };
    }
    this._darkMode = this.storage.getItem(DARK_MODE);
    if (this._darkMode == null) {
      // Let's  figure out what's the system preference then
      const prefersDark = window.matchMedia('(prefers-color-scheme: dark)');
      this._darkMode = prefersDark.matches;
    }
  }

  getDarkMode(): boolean {
    return this._darkMode;
  }

  setDarkMode(darkMode: boolean) {
    if (this._darkMode != darkMode) {
      this.storage.setItem(DARK_MODE, darkMode);
      this._darkMode = darkMode;
    }
  }

  resetAllSettings() {
    // for (const settingName of [DARK_MODE, PRINT_SETTINGS]) {
    //   this.storage.removeItem(settingName);
    // }
    this.storage.factoryReset();
    this.populateSettings();
  }

  getPrintSettings(): PrintSettings {
    return this._printSettings;
  }

  setPrintSettings(printSettings: PrintSettings) {
    this._printSettings = printSettings;
    this.storage.setItem(PRINT_SETTINGS, this._printSettings);
    this.printSettingsChanged.emit(this._printSettings);
  }
}
