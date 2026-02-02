import {doSimpleFiltering,} from 'src/utils/filtering';

export const prepareValue = value => value;

export const doFiltering = (data, filters) => doSimpleFiltering(data, filters);
