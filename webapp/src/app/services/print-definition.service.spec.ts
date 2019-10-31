import { TestBed } from '@angular/core/testing';

import { PrintDefinitionService } from './print-definition.service';

describe('PrintDefinitionService', () => {
  beforeEach(() => TestBed.configureTestingModule({}));

  it('should be created', () => {
    const service: PrintDefinitionService = TestBed.get(PrintDefinitionService);
    expect(service).toBeTruthy();
  });
});
