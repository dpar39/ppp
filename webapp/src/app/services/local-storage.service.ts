import { Injectable } from '@angular/core';

@Injectable({ providedIn: 'root' })
export class LocalStorageService {
  getItem(key: string): any {
    const objString = localStorage.getItem(key);
    if (!objString) {
      return null;
    }
    const obj = JSON.parse(objString);
    return obj.value;
  }

  setItem(key: string, value: any) {
    const obj = { value: value };
    const objString = JSON.stringify(obj);
    localStorage.setItem(key, objString);
  }

  removeItem(key: string): void {
    localStorage.removeItem(key);
  }
}
