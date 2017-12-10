import { TestBed, async } from '@angular/core/testing';

import { AppComponent } from './app.component';
import { MatCardModule, MatButtonModule } from '@angular/material';
import { LandmarkEditorComponent } from './landmark-editor/landmark-editor.component';
import { PassportStandardSelectorComponent } from './passport-standard-selector/passport-standard-selector.component';
import { HttpModule } from '@angular/http';
import { BackEndService } from './services/back-end.service';

describe('AppComponent', () => {
  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        AppComponent,
        LandmarkEditorComponent,
        PassportStandardSelectorComponent
      ],
      imports: [
        HttpModule,
        MatCardModule,
        MatButtonModule
      ],
      providers: [
        BackEndService
      ]
    }).compileComponents();
  }));

  it('should create the app', async(() => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app).toBeTruthy();
  }));

  it(`should have as title 'app'`, async(() => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app.title).toEqual('app');
  }));

  it('should render title in a h1 tag', async(() => {
    const fixture = TestBed.createComponent(AppComponent);
    fixture.detectChanges();
    const compiled = fixture.debugElement.nativeElement;
    expect(compiled.querySelector('h1').textContent).toContain('Welcome to app!');
  }));
});
