const {
    validateIsDefined,
    validateBoolean,
    validateString,
    validateNumber,
    validateInteger,
    validateObject,
    validateArray,
    validateFunction,
    validateTimestamp,
    validateDate,
    validatePlan,
    validateTestResults,
    validateSchema
} = require('../../utils/validators');
const Joi = require('joi');

describe('Validators Tests', () => {

    describe('validateIsDefined', () => {
        test('should return true for defined values', () => {
            expect(validateIsDefined(0)).toBe(true);
            expect(validateIsDefined('')).toBe(true);
            expect(validateIsDefined(null)).toBe(true);
            expect(validateIsDefined(false)).toBe(true);
        });

        test('should return false for undefined', () => {
            expect(validateIsDefined(undefined)).toBe(false);
        });
    });

    describe('validateBoolean', () => {
        test('should return true for boolean values', () => {
            expect(validateBoolean(true)).toBe(true);
            expect(validateBoolean(false)).toBe(true);
        });

        test('should return false for non-boolean values', () => {
            expect(validateBoolean(1)).toBe(false);
            expect(validateBoolean('true')).toBe(false);
            expect(validateBoolean(null)).toBe(false);
        });
    });

    describe('validateString', () => {
        test('should return true for valid strings', () => {
            expect(validateString('hello')).toBe(true);
            expect(validateString('hello', false)).toBe(true);
        });

        test('should return false for empty strings when checking', () => {
            expect(validateString('')).toBe(false);
            expect(validateString('', true)).toBe(false);
        });

        test('should return true for empty strings when not checking', () => {
            expect(validateString('', false)).toBe(true);
        });

        test('should return false for non-strings', () => {
            expect(validateString(123)).toBe(false);
            expect(validateString(null)).toBe(false);
        });
    });

    describe('validateNumber', () => {
        test('should return true for valid numbers', () => {
            expect(validateNumber(123)).toBe(true);
            expect(validateNumber(0)).toBe(true);
            expect(validateNumber(-123)).toBe(true);
            expect(validateNumber(123.45)).toBe(true);
        });

        test('should return false for invalid numbers', () => {
            expect(validateNumber(NaN)).toBe(false);
            expect(validateNumber('123')).toBe(false);
            expect(validateNumber(null)).toBe(false);
        });
    });

    describe('validateInteger', () => {
        test('should return true for integers', () => {
            expect(validateInteger(123)).toBe(true);
            expect(validateInteger(0)).toBe(true);
            expect(validateInteger(-123)).toBe(true);
        });

        test('should return false for non-integers', () => {
            expect(validateInteger(123.45)).toBe(false);
            expect(validateInteger('123')).toBe(false);
            expect(validateInteger(null)).toBe(false);
        });
    });

    describe('validateObject', () => {
        test('should return true for valid objects', () => {
            expect(validateObject({})).toBe(true);
            expect(validateObject({ key: 'value' })).toBe(true);
            expect(validateObject([], false)).toBe(true);
        });

        test('should handle empty check', () => {
            expect(validateObject({}, true)).toBe(false);
            expect(validateObject({ key: 'value' }, true)).toBe(true);
        });

        test('should return false for non-objects', () => {
            expect(validateObject(null)).toBe(false);
            expect(validateObject('string')).toBe(false);
            expect(validateObject(123)).toBe(false);
        });
    });

    describe('validateArray', () => {
        test('should return true for valid arrays', () => {
            expect(validateArray([1, 2, 3])).toBe(true);
            expect(validateArray([], false)).toBe(true);
        });

        test('should handle empty check', () => {
            expect(validateArray([])).toBe(false);
            expect(validateArray([1], true)).toBe(true);
        });

        test('should return false for non-arrays', () => {
            expect(validateArray({})).toBe(false);
            expect(validateArray('string')).toBe(false);
        });
    });

    describe('validateFunction', () => {
        test('should return true for functions', () => {
            expect(validateFunction(() => {})).toBe(true);
            expect(validateFunction(function() {})).toBe(true);
        });

        test('should return false for non-functions', () => {
            expect(validateFunction({})).toBe(false);
            expect(validateFunction('string')).toBe(false);
        });
    });

    describe('validateTimestamp', () => {
        test('should return true for valid timestamps', () => {
            expect(validateTimestamp(Date.now())).toBe(true);
            expect(validateTimestamp(1571048280)).toBe(true);
        });

        test('should return false for invalid timestamps', () => {
            expect(validateTimestamp(1571048279)).toBe(false);
            expect(validateTimestamp('123')).toBe(false);
            expect(validateTimestamp(123.45)).toBe(false);
        });
    });

    describe('validateDate', () => {
        test('should return true for valid dates', () => {
            expect(validateDate('2023-01-01')).toBe(true);
            expect(validateDate('2023-01-01T10:00:00Z')).toBe(true);
        });

        test('should return false for invalid dates', () => {
            expect(validateDate('invalid-date')).toBe(false);
            expect(validateDate('1970-01-01')).toBe(false); // Too old
        });
    });

    describe('validatePlan', () => {
        const validPlan = [
            {
                id: 0,
                type: 'charge',
                isCollapsed: false,
                source: 'internal'
            }
        ];

        test('should return valid plan for correct input', () => {
            const result = validatePlan(validPlan, false);
            expect(result).toBeDefined();
            expect(Array.isArray(result)).toBe(true);
        });

        test('should throw error for mandatory invalid plan', () => {
            expect(() => validatePlan(null, true)).toThrow('Invalid plan.');
            expect(() => validatePlan([], true)).toThrow('Invalid plan.');
        });

        test('should handle non-mandatory invalid plan', () => {
            const result = validatePlan(null, false);
            expect(result).toBeUndefined();
        });

        test('should filter out invalid properties', () => {
            const planWithExtra = [
                {
                    id: 0,
                    type: 'charge',
                    invalidProp: 'should be removed'
                }
            ];
            const result = validatePlan(planWithExtra, false);
            expect(result[0]).not.toHaveProperty('invalidProp');
        });
    });

    describe('validateTestResults', () => {
        const validTestResults = [
            {
                timestamp: 100,
                voltage: 3.7,
                temperature: 25,
                current: 1.5
            }
        ];

        test('should return valid test results for correct input', () => {
            const result = validateTestResults(validTestResults, false);
            expect(result).toBeDefined();
            expect(Array.isArray(result)).toBe(true);
        });

        test('should throw error for mandatory invalid test results', () => {
            expect(() => validateTestResults(null, true)).toThrow('Invalid TestResults.');
            expect(() => validateTestResults([], true)).toThrow('Invalid TestResults.');
        });

        test('should handle non-mandatory invalid test results', () => {
            const result = validateTestResults(null, false);
            expect(result).toBeUndefined();
        });
    });

    describe('validateSchema', () => {
        const schema = Joi.object({
            name: Joi.string().required(),
            age: Joi.number().min(0)
        });

        test('should pass for valid data', () => {
            expect(() => validateSchema(schema, { name: 'John', age: 25 })).not.toThrow();
        });

        test('should throw for invalid data', () => {
            expect(() => validateSchema(schema, { age: 25 })).toThrow();
            expect(() => validateSchema(schema, { name: 'John', age: -1 })).toThrow();
        });

        test('should include context in error message', () => {
            expect(() => validateSchema(schema, { age: 25 }, 'test context')).toThrow();
        });
    });
});