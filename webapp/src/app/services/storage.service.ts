import { Injectable } from '@angular/core';

const dbVersion = 1;
const STORE_NAME = 'fs-store';
const DB_NAME = 'fs-db';

@Injectable({ providedIn: 'root' })
export class StorageService {
  private db: IDBDatabase;

  constructor() {
    var request = indexedDB.open(DB_NAME, dbVersion);
    request.onsuccess = (event) => {
      console.log('Success creating/accessing IndexedDB database');
      this.db = request.result;
      this.db.onerror = (event) => {
        console.log('Error creating/accessing IndexedDB database');
      };
    };

    request.onupgradeneeded = (e) => {
      let db = request.result;
      if (e.oldVersion < 1) {
        db.createObjectStore(STORE_NAME);
      }
    };
  }
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

  async saveFile(filePath: string, data: Blob | ArrayBuffer) {
    return new Promise<void>((accept, reject) => {
      const tx = this.db.transaction([STORE_NAME], 'readwrite');
      const store = tx.objectStore(STORE_NAME);
      const req = store.put(data, filePath);
      req.onsuccess = () => accept();
      req.onerror = () => reject();
    });
  }

  async loadFile(filePath: string): Promise<Blob> {
    return new Promise<Blob>((accept, reject) => {
      const tx = this.db.transaction([STORE_NAME], 'readonly');
      const store = tx.objectStore(STORE_NAME);
      const req = store.get(filePath);
      req.onsuccess = (e) => accept(req.result);
      req.onerror = () => reject();
    });
  }

  async fileExists(filePath: string): Promise<boolean> {
    return new Promise<boolean>((accept, reject) => {
      const tx = this.db.transaction([STORE_NAME], 'readonly');
      const store = tx.objectStore(STORE_NAME);
      const req = store.get(filePath);
      req.onsuccess = (e) => accept(req.result);
      req.onerror = () => reject();
    });
  }

  async factoryReset(): Promise<void> {
    localStorage.clear();
    return new Promise<void>((accept, reject) => {
      const tx = this.db.transaction([STORE_NAME], 'readwrite');
      const store = tx.objectStore(STORE_NAME);
      const req = store.clear();
      req.onsuccess = () => accept();
      req.onerror = () => reject();
    });
  }
}
