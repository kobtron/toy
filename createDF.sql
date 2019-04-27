CREATE TABLE if not exists file (
  id int primary key,
  name varchar not null,
  parent int default 0,
  isdir int default 0,
  data blob
);
