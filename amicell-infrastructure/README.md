####To silently create the S3 busket add "--resolve-s3" flag.
####Dev
`sam build`

`sam deploy --config-env dev --no-confirm-changeset --capabilities CAPABILITY_IAM CAPABILITY_NAMED_IAM --region us-east-1 --profile dev`
