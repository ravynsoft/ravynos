ALTER TABLE applications ADD COLUMN bundleid text;
UPDATE schema SET version=4;
