import React, {forwardRef} from 'react';
import {Link} from 'react-router-dom';

const RouterLink = ({href, ...other}, ref) => <Link ref={ref} to={href} {...other} />;

export default forwardRef(RouterLink);
