import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { IonicModule } from '@ionic/angular';

import { HomePage } from './home.page';
import { LandmarkEditorComponent } from './landmark-editor.component';
import { PhotoStandardSelectorComponent } from './photo-standard-selector.component';
import { PrintDefinitionService } from '../services/print-definition.service';
import { PrintDefinitionSelectorComponent } from './print-definition-selector.component';
import { BackEndService } from '../services/backend.service';
import { PhotoStandardService } from '../services/photo-standard.service';
import { HttpClientModule } from '@angular/common/http';

describe('HomePage', () => {
  let component: HomePage;
  let fixture: ComponentFixture<HomePage>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [HomePage, LandmarkEditorComponent, PhotoStandardSelectorComponent, PrintDefinitionSelectorComponent],
      imports: [IonicModule.forRoot()],
      providers: [HttpClientModule, BackEndService, PhotoStandardService, PrintDefinitionService]
    }).compileComponents();

    fixture = TestBed.createComponent(HomePage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  }));

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
