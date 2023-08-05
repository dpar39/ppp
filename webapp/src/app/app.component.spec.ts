import { CUSTOM_ELEMENTS_SCHEMA } from '@angular/core';
import { TestBed, waitForAsync } from '@angular/core/testing';

import { Platform } from '@ionic/angular';
import { RouterTestingModule } from '@angular/router/testing';

import { AppComponent } from './app.component';
import { ServiceWorkerModule } from '@angular/service-worker';
import { HttpClientModule } from '@angular/common/http';
import { BrowserModule } from '@angular/platform-browser';
import { AppRoutingModule } from './app-routing.module';

describe('AppComponent', () => {
  let platformReadySpy, platformSpy;

  beforeEach(waitForAsync(() => {
    platformReadySpy = Promise.resolve();
    platformSpy = jasmine.createSpyObj('Platform', { ready: platformReadySpy });

    TestBed.configureTestingModule({
      declarations: [AppComponent],
      schemas: [CUSTOM_ELEMENTS_SCHEMA],
      providers: [{ provide: Platform, useValue: platformSpy }],
      imports: [
        HttpClientModule,
        BrowserModule,
        AppRoutingModule,
        RouterTestingModule.withRoutes([]),
        ServiceWorkerModule.register('', { enabled: false }),
      ],
    }).compileComponents();
  }));

  it('should create the app', async () => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app).toBeTruthy();
  });

  it('should initialize the app', async () => {
    TestBed.createComponent(AppComponent);
    expect(platformSpy.ready).toHaveBeenCalled();
    await platformReadySpy;
  });

  it('should have menu labels', async () => {
    const fixture = await TestBed.createComponent(AppComponent);
    await fixture.detectChanges();
    const app = fixture.nativeElement;
    const menuItems = app.querySelectorAll('ion-label');

    const expecteMenuItems = ['Start', 'Settings', 'App Info'];

    expect(menuItems.length).toEqual(expecteMenuItems.length);
    let i = 0;
    for (const menuItemText of expecteMenuItems) {
      expect(menuItems[i++].textContent).toContain(menuItemText);
    }
  });

  it('should have urls', async () => {
    const fixture = await TestBed.createComponent(AppComponent);
    await fixture.detectChanges();
    const app = fixture.nativeElement;
    const menuItems = app.querySelectorAll('ion-item');

    const expectedRoutes = ['/start', '/settings', '/about'];
    expect(menuItems.length).toEqual(expectedRoutes.length);

    let i = 0;
    for (const expectedRoute of expectedRoutes) {
      expect(menuItems[i++].getAttribute('ng-reflect-router-link')).toEqual(expectedRoute);
    }
  });
});
